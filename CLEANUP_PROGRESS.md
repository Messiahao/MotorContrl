# 代码精简进度

- 当前编译配置：`USE_HAL_DRIVER`、`STM32F103xE`；未定义 `DEBUG`、`TEST`、`TRACE`、`LOG`、`DBG`。
- 处理范围：自研板级驱动、应用层和工程入口；`MotorContrl/Drivers/STM32F1xx_HAL_Driver/`、`MotorContrl/Drivers/CMSIS/`、`MotorContrl/MDK-ARM/RTE/` 为 vendor/生成内容，不纳入源码精简。
- 当前验证基线：Keil Arm Compiler 6.24 全量构建通过（0 errors，0 warnings，`Code=17970`、`RO-data=650`、`RW-data=12`、`ZI-data=2444`）；结构冻结、三轴专项、默认配置 50 组和 `continuous_disabled` 2 组主机黄金回归均通过。硬件电气和实时性仍待实机验证。
- 当前状态：清单内 24 个模块均已处理完成，无待确认项。
- 恢复规则：每次开始前处理表中第一个 `⏳ 进行中`；若没有，则处理第一个 `⏸️ 未开始`。先解决该模块记录中的“待确认”项。

| 模块 | 状态 | 删除符号数 | 优化逻辑数 | 备注 |
|------|------|-----------:|-----------:|------|
| `MotorContrl/Drivers/Board/led.c` + `MotorContrl/Inc/led.h` | ✅ 已完成 | 0 | 0 | 无可安全精简项；Keil 构建通过 |
| `MotorContrl/Inc/config.h` + `MotorContrl/Inc/app_types.h` | ✅ 已完成 | 0 | 2 | 删除无效 C++ 链接包装；Keil 构建通过 |
| `MotorContrl/Src/system_clock.c` + `MotorContrl/Inc/system_clock.h` | ✅ 已完成 | 0 | 0 | 回归脚本要求保留结构；Keil 构建通过 |
| `MotorContrl/Drivers/Board/gpio.c` + `MotorContrl/Inc/gpio.h` | ✅ 已完成 | 0 | 0 | 回归脚本要求保留 MX 初始化；Keil 构建通过 |
| `MotorContrl/Drivers/Board/i2c.c` + `MotorContrl/Inc/i2c.h` | ✅ 已完成 | 0 | 0 | 回归脚本冻结初始化/MSP；Keil 构建通过 |
| `MotorContrl/Drivers/Board/spi.c` + `MotorContrl/Inc/spi.h` | ✅ 已完成 | 0 | 0 | 回归脚本冻结 MX 初始化；Keil 构建通过 |
| `MotorContrl/Drivers/Board/tim.c` + `MotorContrl/Inc/tim.h` | ✅ 已完成 | 0 | 0 | 时序/ISR 与冻结结构均保留；Keil 构建通过 |
| `MotorContrl/Drivers/Board/usart.c` + `MotorContrl/Inc/usart.h` | ✅ 已完成 | 0 | 0 | 无安全可删项；构建/冻结结构通过，既有行为基线差异未变 |
| `MotorContrl/Drivers/Board/mcp4728.c` + `MotorContrl/Inc/mcp4728.h` | ✅ 已完成 | 0 | 1 | 删除冗余码值掩码；构建及 4096 码值穷举等价检查通过 |
| `MotorContrl/Drivers/Board/tmc5160.c` + `MotorContrl/Inc/tmc5160.h` | ✅ 已完成 | 0 | 0 | SPI/寄存器时序及公开接口均保留；构建与冻结检查通过 |
| `MotorContrl/APPs/app_led.c` + `MotorContrl/Inc/app_led.h` | ✅ 已完成 | 0 | 3 | 合并状态映射与初始化表达式，减少 13 行；构建通过 |
| `MotorContrl/APPs/app_aux_output.c` + `MotorContrl/Inc/app_aux_output.h` | ✅ 已完成 | 0 | 2 | 合并双路重复校验，减少 14 行/40 B；构建与穷举检查通过 |
| `MotorContrl/APPs/app_light.c` + `MotorContrl/Inc/app_light.h` | ✅ 已完成 | 1 | 2 | 删除一次性码值变量并合并错误出口，减少 6 行；专用用例通过 |
| `MotorContrl/APPs/app_limit.c` + `MotorContrl/Inc/app_limit.h` | ✅ 已完成 | 5 | 0 | 删除调试/只写状态，减少 18 行、88 B Code、24 B ZI；三轴用例通过 |
| `MotorContrl/APPs/app_motion.c` + `MotorContrl/Inc/app_motion.h` | ✅ 已完成 | 0 | 0 | 时序/安全路径保留；三轴回归与 Keil 构建通过 |
| `MotorContrl/APPs/app_protocol.c` + `MotorContrl/Inc/app_protocol.h` | ✅ 已完成 | 9 | 3 | 删除纯调试协议状态；-324 B Code/-24 B ZI，构建与三轴回归通过 |
| `MotorContrl/APPs/app_self_test.c` + `MotorContrl/Inc/app_self_test.h` | ✅ 已删除 | — | — | 旧 X 轴自动上电自检为测试专用，正常三轴运动检查已由 AppMotion 公共路径覆盖 |
| `MotorContrl/APPs/app_scheduler.c` + `MotorContrl/Inc/app_scheduler.h` | ✅ 已完成 | 1 | 3 | 合并命令分发、删除空初始化调用；-28 B Code，构建/专项用例通过 |
| `MotorContrl/Src/stm32f1xx_hal_msp.c` | ✅ 已完成 | 0 | 0 | CubeMX MSP/SWD 配置保留；完整构建与冻结结构通过 |
| `MotorContrl/Src/stm32f1xx_it.c` + `MotorContrl/Inc/stm32f1xx_it.h` | ✅ 已完成 | 0 | 0 | 向量/ISR/HAL 转发全部有效；冻结结构与完整构建通过 |
| `MotorContrl/Src/main.c` + `MotorContrl/Inc/main.h` | ✅ 已完成 | 0 | 1 | ISR 状态收紧为文件内链接；-1 行/-200 B Code/-72 B ZI，Keil 构建通过 |
| `MotorContrl/Inc/stm32f1xx_hal_conf.h` | ✅ 已完成 | 34 | 1 | 删除重复 GPIO 开关及未启用 ETH 的 33 个模板宏；-57 行，Keil 构建通过 |
| `MotorContrl/Src/system_stm32f1xx.c` | ✅ 已完成 | 0 | 0 | CMSIS 启动/时钟接口均有效；源码不改，Keil 构建通过 |
| `tests/` 回归基础设施 | ✅ 已完成 | 7 | 4 | 同步 USART2/初始化结构；52 组黄金回归及三轴专项通过，固件资源不变 |
