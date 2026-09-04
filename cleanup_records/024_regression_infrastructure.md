### 模块：`tests/` 回归基础设施

- 扫描时间：2026-09-04 14:51:25 +08:00
- 当前条件编译配置：固件定义 `USE_HAL_DRIVER`、`STM32F103xE`；未定义 `DEBUG`、`TEST`、`TRACE`、`LOG`、`DBG`。主机回归默认使用 `SERIAL_MOTION_VALIDATION_COMMAND_ENABLED=1U`、`Z_LIMIT_PROTECTION_ENABLED=1U`、X/Y/Z 三轴；另编译 `continuous_disabled` 配置，将持续运动验证命令关闭。
- 静态扫描：
  - `tests/check_refactor.py` 全局对象：`ROOT`、`FW`、`TESTS`、`REMOVED_LEGACY_STATE`；函数：`mask`、`function`、`body`、`no_includes`、`text`、`tokens`、`run_host_executable`、`check_structure`、`generate`、`main`。这些对象均由脚本入口、结构检查或主机源码生成路径引用。
  - `tests/refactor_mock.c` 对生成测试单元可见的对象：`huart2`、`htim2`、`htim3`、`htim4`、`SystemClock_Config`，以及生产代码使用的 HAL/BSP 模拟入口。
  - `tests/refactor_mock.c` 文件内状态：`ports`、`uart_instance`、`tick`、`input_levels`、`output_levels`、`registers_tmc`、`registers_tmc_y`、`registers_tmc_z`、`period`、`compare_value`、`counter`、`interrupt_enable`、`pwm_running`、`fault`、`tx_failure`、`irq_after_gpio`、`pulse_direction`、`pwm_axis`、`pwm_channel`、`step_mode_port`、`step_mode_pin`、`step_mode`、`ore`、`rx_read`、`rx_count`、`rx_data`、`i2c_address`、`i2c_data`、`i2c_length`、`i2c_write_count`、`trace_hash`、`event_count`。
  - `tests/refactor_mock.c` 文件内函数：`Trace`、`MockPulse`、`HAL_GetTick`、`HAL_Delay`、`HAL_GPIO_ReadPin`、`HAL_GPIO_WritePin`、`HAL_GPIO_Init`、四个 `HAL_NVIC_*` 模拟函数、`HAL_TIM_PWM_Start`、`HAL_TIM_PWM_Stop`、`__get_PRIMASK`、`__disable_irq`、`__enable_irq`、`MockUartFlag`、`MockClearOre`、`HAL_UART_Transmit`、`MockTmcReadAxis`、`MockTmcWriteAxis`、`HAL_Init`、`MockInit`、`BspI2c_Write`；`HAL_TIM_PeriodElapsedCallback` 为生产代码提供的跨片段声明。
  - `tests/refactor_mock.c` 宏：GPIO 端口/模式/引脚、TIM3/TIM4 实例、TIM/EXTI IRQ、TIM 通道/标志/中断、UART 标志、`RESET`，以及 `__HAL_TIM_*`、`__HAL_GPIO_EXTI_CLEAR_IT`、`__HAL_UART_*` 模拟宏。逐项均由当前生产源码、三轴断言或模拟函数引用。
  - `tests/refactor_cases.c` 函数：`Queue`、`Poll`、`Command`、`LightCommand`、`main`；`main` 内静态查表 `speeds`、`distances`、`p`、`b`、`axes`、`channels`、`dir_ports`、`dir_pins`、`step_ports`、`step_pins` 均由对应场景循环读取。
  - 夹具：`pre_refactor.zip` 只冻结启动、ISR、vendor 哈希、工程选项和模块边界；`current_behavior.json` 保存当前 52 个主机行为摘要。两者职责已分离。
  - 调试属性：未发现可进入固件的调试变量、打印缓存或 DEBUG 分支；`trace_hash`、`event_count` 和 `Snapshot` 输出是主机黄金回归的判定数据，不属于可删除调试残留。
- 引用分析：
  - `tests/` 未列入 Keil 工程，不参与目标固件编译，因此所有改动不改变 MCU 映像、ISR 或硬件时序。
  - 旧运行期差分分支依赖已删除的 USART3、轮询限位和旧状态，无法代表当前批准行为；保留它会在 `MX_USART3_UART_Init` 处提前失败，或把正确的三轴/EXTI 行为误报为差异。
  - 当前脚本从现有头文件、板级驱动、应用模块和 `main.c` 组合单一主机测试单元；默认 50 个场景和关闭持续运动命令的 2 个场景均与黄金摘要逐字节哈希/长度匹配。
  - `case 49` 直接检查 X/Y/Z 轴选择、TIM/GPIO 路由、完成/停止、加减速和限位隔离，避免仅依赖摘要掩盖三轴路由错误。
  - 所有模拟状态均存在读写路径；未发现仅写不读、仅读不写或未调用的剩余私有函数。
- 删除的变量/宏/函数：
  - `SPI_HandleTypeDef`、`hspi1`：旧基线 SPI 包装使用，当前 BSP 生成路径不再引用。
  - `huart3`：旧调试串口对象；替换为当前主通信对象 `huart2`。
  - `TIM2`、`HAL_MAX_DELAY`：旧基线生成分支使用，当前测试单元未引用。
  - `MockTmcRead`、`MockTmcWrite`：仅服务已删除的单轴旧基线包装；当前统一使用带轴参数的 `MockTmcReadAxis`、`MockTmcWriteAxis`。
- 合并/简化的逻辑：
  - 删除“旧源码 + 当前源码”双编译、双执行和逐行比较分支，只编译当前源码并与固定黄金摘要比较。
  - 将重构前 ZIP 的用途收窄到仍然有效的结构不变量；新增紧凑的 `current_behavior.json` 保存当前运行期基线。
  - 同步结构锁：要求 USART2/PA2/PA3 和当前调度器初始化顺序存在，同时禁止 USART3/`huart3`/`BspUsart3_Init` 回流。
  - 初始化顺序由结构断言独立验证，运行期轨迹从 `TestInit` 后开始；原自检标签改为中性的长空闲标签。
- 保留但标记为“待确认”的项：无。
- 修改后代码行数变化：可执行测试源码净减 12 行（736 行 → 724 行）；新增 58 行黄金数据后，`tests/` 文本总计净增 46 行。
- 编译检查：通过。MSVC C11 `/W3` 编译默认及 `continuous_disabled` 两种主机测试单元，无编译诊断；断线前 Keil Arm Compiler 6.24 全量构建为 0 errors、0 warnings。
- 结构/行为检查：结构冻结通过；三轴专项通过；默认 50 组 + `continuous_disabled` 2 组，共 52 组黄金回归通过。明细位于 `tmp/refactor_host_tests/report.json`。
- 资源结果：测试目录不参与目标编译，保持 `Code=17970`、`RO-data=650`、`RW-data=12`、`ZI-data=2444`。
- 待办事项：
  - [ ] 主机回归无法验证 GPIO 电气、电机负载和中断响应时间；发布前仍需按 `doc/refactor_test_plan.md` 完成实机测试。

#### Diff

```diff
- 生成并运行重构前基线与当前源码，再逐行比较输出
+ 仅生成当前源码，比较 52 个固定 SHA-256/字节数黄金摘要

- UART_HandleTypeDef huart3;
- SPI_HandleTypeDef hspi1;
- MockTmcRead(...)/MockTmcWrite(...)
+ UART_HandleTypeDef huart2;
+ 三轴统一使用 MockTmcReadAxis(...)/MockTmcWriteAxis(...)

+ 结构锁定 USART2、调度器初始化顺序和三轴 EXTI 配置
+ tests/fixtures/current_behavior.json
```
