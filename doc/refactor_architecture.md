# 固件分层重构说明

日期：2026-08-27。对象：STM32F103RCT6，CN4/USART3 主机通信，CN6/USART2 扩展保留。

## 范围与基线

本次在用户已有、尚未提交的工作区上做代码提取，保留其中的 X 轴加减速实现，不以 Git HEAD 覆盖工作区。重构前 `Src/main.c` 的 SHA-256：

`30E1BDB82DAF4A144D0FCE1BEB848B64DF9170E95988D86E613FF6184D28BE41`

基线源文件保存在 `tests/fixtures/pre_refactor.zip`，完整本地编译副本保存在 `tmp/refactor_baseline_20260827/`。它们是对照材料，不是新的发布版本。

灯板这次只搭接口，不实现 `05 00` 命令、不访问 DAC、不写 EEPROM、不修改 I2C 电平处理方案。CN1 的既有 5 V 继电器输出继续工作；CN10 的四路 DAC 输出暂不纳入“已实现灯板功能”。

## 模块划分清单

| 层次 | 文件位置 | 职责 |
| --- | --- | --- |
| 系统 | `Src/main.c` | 系统启动、模块初始化入口、固定轮询入口；保留冻结的 TIM2 回调及其计算函数。 |
| 系统 | `Src/system_clock.c` | 原系统时钟配置、Error_Handler、断言回调；函数体不变。 |
| 系统 | `Src/stm32f1xx_it.c` | 原 ISR，文件逐字节不变。 |
| 系统 | `Src/stm32f1xx_hal_msp.c`、`system_stm32f1xx.c` | 原 HAL 基础初始化及系统启动支持，不变。 |
| 驱动 | `Drivers/Board/gpio.c` + `Inc/gpio.h` | 原 GPIO 初始化、引脚读写、X STEP 模式切换、即时读取三路 X 限位。 |
| 驱动 | `Drivers/Board/usart.c` + `Inc/usart.h` | 原 USART2/3 初始化；CN4 溢出清除、RXNE 读取、DR 读取与阻塞发送。 |
| 驱动 | `Drivers/Board/spi.c` + `Inc/spi.h` | 原 SPI1 初始化与读写包装；模式、分频和超时不变。 |
| 驱动 | `Drivers/Board/tim.c` + `Inc/tim.h` | 原 TIM2/3/4 初始化；前台的 TIM2 PWM 启停及中断配置包装。 |
| 驱动 | `Drivers/Board/i2c.c` + `Inc/i2c.h` | 原 I2C1 初始化，100 kHz、PB8/PB9 不变；预留总线读写接口，当前灯板不会调用。 |
| 驱动 | `Drivers/Board/led.c` + `Inc/led.h` | PC14 LED 的电平输出。 |
| 驱动 | `Drivers/Board/tmc5160.c` + `Inc/tmc5160.h` | 原 TMC5160 寄存器读写、片选、使能及原初始化函数。 |
| 驱动占位 | `Drivers/Board/mcp4728.c` + `Inc/mcp4728.h` | 空 Init、未实现的 Read/Write；不发 I2C、不改变调用者读缓冲区。 |
| 调度 | `APPs/app_scheduler.c` | 按固定顺序调用应用，逐帧分发命令，持有私有应用状态。 |
| 业务 | `APPs/app_protocol.c` | UART 接收、14 字节组帧、原链路测试命令；每帧立即交回调度器分发。 |
| 业务 | `APPs/app_motion.c` | 运动命令校验、应答、状态查询、准备启动、停止、完成处理。 |
| 业务 | `APPs/app_aux_output.c` | `03 00` 继电器、`04 00` 抱闸控制；校验和回传规则不变。 |
| 业务 | `APPs/app_limit.c` | 原 10 ms 九路限位采样和调试掩码更新。 |
| 业务 | `APPs/app_led.c` | 原 500 ms LED 闪烁逻辑。 |
| 业务 | `APPs/app_self_test.c` | 原条件编译自检顺序；默认仍关闭，不删除旧自检。 |
| 业务占位 | `APPs/app_light.c` | 空 Init、未实现的 Process；协议 1～4 路映射 A～D 的约定写入头文件。 |
| 接口/配置 | `Inc/app_*.h`、`Inc/config.h` | 函数声明、参数类型、集中配置；没有业务变量定义或 extern 业务变量。 |

`Drivers/STM32F1xx_HAL_Driver` 和 `Drivers/CMSIS` 保持原样。没有运行 CubeMX，也没有改 `.ioc`。

注意：CubeMX 仍按原工程生成 `Src` 外设文件。后续若修改 `.ioc`，应审查生成差异并合并到 `Drivers/Board`，不能直接覆盖已提取的应用或把两份同名驱动一起加入工程。

## 初始化与执行顺序

`main()` 仅包含以下三个部分：

```c
HAL_Init();
SystemClock_Config();

AppScheduler_Init(&motion_irq);

AppScheduler_Run();
```

原硬件初始化顺序完整保留：

```text
HAL_Init → SystemClock_Config
→ GPIO → USART2 → I2C1 → SPI1 → TIM2 → TIM3 → TIM4 → USART3
→ LED 时间戳/关闭 LED → XYZ EN 禁用 → XYZ DIR 拉低
→ 限位采样时间戳 → 自检时间戳及 pending → 原 build marker
→ 无硬件副作用的辅助/灯板占位 Init
```

没有为了“所有模块 Init”而新增调用 `BspTmc5160_Init()`：原程序没有调用它，它含有使能动作，新增调用会改变上电行为。

轮询顺序：

```text
AppLed_Process
→ AppProtocol_Process：先检查 ORE，再连续读取到 RXNE 清空
   → 每个非链路测试帧立即回到调度器
      → 运动 / 继电器抱闸业务处理并在原位置回传
→ AppLimit_Process：消费九路限位 EXTI 快照并更新调试状态
→ AppMotion_Process：消费限位事件 → 停止 → 待启动 → 完成
→ 原宏条件允许时，执行 AppSelfTest_Process
```

运动任务中的三个处理分支仍是独立 `if`，没有改成 `else if`。启动前仍直接读取限位电平；运行中由九路 EXTI 上升沿在 ISR 中停止 X 轴定时器和驱动，主循环负责状态清理及应答。没有新增 RTOS、消息队列、软件定时器、UART 中断或 DMA。

原 100 ms、200 ms、1 ms 阻塞等待保留在原有操作之间，均已标注“阻塞延时，建议后续改为状态机定时器替代”。原来未调用的 TMC 初始化中的 10 ms 等待也保留并标注。

## 参数传递与封装

- 公共应用接口统一为 `App<模块>_Init/Process...`，板级接口为 `Bsp<外设>_Init/Read.../Write...`。生成器的 `MX_*` 实现改为驱动文件内的 `static` 函数，HAL 要求的回调保留原名。
- 调度器私有 `runtime` 持有 Protocol/Motion/Aux/Limit/Tmc 状态，调用各模块时显式传入指针。应用模块之间不直接调用；串口模块的帧回调只返回调度器。
- 原 `volatile` 字段保留 `volatile`；不把 ISR 状态复制成快照，不增加“读取一次”的缓存。原来重复读取的脉冲计数等字段仍逐次读取。
- `main.c` 中保留 15 个 ISR 使用的原变量；前台通过 `AppMotionIrq` 中的地址访问它们，未新增 `extern` 业务变量。原速度计算仅通过一个前台桥接函数供启动流程调用，ISR 本身不经过该函数指针。
- `Inc/app_types.h` 只有类型声明，不分配变量存储。UART/SPI/I2C 和 TIM3/4 句柄已隐藏为驱动私有状态。

### 必须保留的 ISR / HAL 例外

“ISR 不改不移动”优先于把 `main.c` 清到只有三部分：三部分要求体现在 `main()`，但 `main.c` 仍保留 `HAL_TIM_PeriodElapsedCallback`、`Serial_Motion_IntegerSqrt`、`Serial_Motion_ProfileSpeed`、`Serial_Motion_ApplySpeed` 四个原函数。

同样，`tim.h` 保留唯一的 `extern TIM_HandleTypeDef htim2;` **声明**，以满足原样保留的 `TIM2_IRQHandler` 和回调调用链；不是头文件变量定义，也不用于应用模块通信。禁止为了隐藏它而把 ISR 中的 `&htim2` 改成访问函数或宏。

ISR 调用链中的原数字常量也保持原文；其他具有业务含义的协议值、字段位置、阈值、时间、寄存器参数及位移已归入头文件。零初始化和布尔赋值不是新增配置项。

## 灯板占位边界

当前调用：启动时仅调用空的 `BspMcp4728_Init()` 和 `AppLight_Init()`，轮询不额外增加灯板任务。

| 接口 | 当前行为 | 后续填写内容 |
| --- | --- | --- |
| `AppLight_Init()` | 无操作 | 确认上电输出策略后再填写。 |
| `AppLight_Process(protocol_channel, enabled)` | 返回 `APP_LIGHT_NOT_IMPLEMENTED` | 校验协议通道/动作，映射 DAC 通道和码值，调用 DAC 驱动。 |
| `BspMcp4728_Init()` | 无操作 | 验证地址、参考源、LDAC 与上电配置。 |
| `BspMcp4728_Read(channel, code)` | 返回未实现，`*code` 不变 | 真实 I2C 读取与解析。 |
| `BspMcp4728_Write(channel, code)` | 返回未实现 | 真实 I2C 易失寄存器写入。 |

后续将 `05 00` 分支接入 `Scheduler_ProcessFrame`，不能在驱动未实现时发送“灯已打开”成功应答。当前该命令仍走原未知命令路径：不回传成功、不驱动任何输出。

MCP4728 5 V 供电而 SCL/SDA 上拉 3.3 V 的电气风险暂缓处理，不代表已验证。当前也不主动清零 DAC，因此 CN10 上电电压可能来自 EEPROM 中的历史配置，不能把它当作空接口的执行结果。

## 验证边界

重构前后均使用原 Keil Arm Compiler 6.24、原优化等级构建。ISR 源码和优先级不变，但编译器因编译单元/数据布局变化重新生成机器码；不能据此承诺逐周期耗时相同。当前 TIM2 回调代码节由 532 字节变为 500 字节，该差异必须结合实机最高速波形与延迟测试评估。

主机差分测试比较真实 C 业务代码在相同模拟输入下的有序 HAL 调用和全部原状态值。它不模拟电气行为、实际 CPU 周期或任意指令处的中断抢占，不代替上板验收。详见 [逐项测试方案](refactor_test_plan.md)。
