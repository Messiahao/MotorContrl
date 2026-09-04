### 模块：`MotorContrl/Src/main.c` + `MotorContrl/Inc/main.h`

- 扫描时间：2026-09-04 13:44:50 +08:00
- 当前条件编译配置：Keil AC6.24，定义 `USE_HAL_DRIVER`、`STM32F103xE`；未定义 `DEBUG`、`TEST`、`TRACE`、`LOG`、`DBG`。`Z_LIMIT_PROTECTION_ENABLED=0`，但 PA10/PA11/PA12 的 Z 轴限位映射和 EXTI 路径保持不变。
- 静态扫描：
  - ISR 共享变量共 16 个：`serial_motion_active`、`serial_motion_done`、`serial_motion_axis`、`serial_motion_continuous`、`serial_motion_pulses_done`、`serial_motion_target_steps`、`serial_motion_target_speed_hz`、`serial_motion_current_speed_hz`、`serial_motion_peak_speed_hz`、`serial_motion_profile_phase`、`serial_motion_profile_accel_steps`、`serial_motion_profile_cruise_steps`、`serial_motion_profile_decel_steps`、`serial_motion_profile_update_count`、`serial_motion_last_period_ticks`、`serial_motion_profile_error`。
  - 文件内函数：`Serial_Motion_IntegerSqrt`、`Serial_Motion_ProfileSpeed`、`Serial_Motion_ApplySpeed`、`Main_WriteInitialSpeed`，均有调用。
  - 对外/框架入口：`main`、`HAL_TIM_PeriodElapsedCallback`、`HAL_GPIO_EXTI_Callback`，分别由启动代码或 HAL 弱回调机制使用。
  - `main.h` 包含板级 GPIO 映射、`Error_Handler` 声明及 CubeMX 保护区；未发现调试变量、打印缓冲、测试宏或未调用函数。
- 引用分析：
  - 16 个运动状态量均由冻结 ISR 路径使用，并全部通过 `AppMotionIrq` 地址表交给前台运动模块；无变量可删除。
  - 工程中没有这些变量的 `extern` 声明或跨翻译单元直接引用，因此可收紧为文件内链接，不改变 `volatile`、对象地址或 ISR/前台共享方式。
  - `gpio.h` 未提供本文件实际使用的独有声明；GPIO/HAL 类型由 `main.h` 提供，限位回调由 `app_limit.h` 提供。
  - `main.h` 的九路限位、三轴定时器、SPI/I2C/UART 和 SWD 引脚与 `doc/schematic_info.md` 一致；未改硬件映射。
- 删除的变量/宏/函数：无。
- 删除的无用 include：`#include "gpio.h"`，原因：本文件无直接依赖。
- 合并/简化的逻辑：将 16 个仅限本文件使用的 ISR 状态量改为 `static volatile`，消除不必要的外部链接；链接器因此可完成更强的内部优化。
- 保留但标记为“待确认”的项：无。
- 修改后代码行数变化：-1 行（`.c` 261 行 → 260 行；`.h` 130 行不变）。
- 编译检查：通过。Keil Arm Compiler 6.24 增量构建 0 errors、0 warnings。
- 结构/行为检查：运动曲线和两个 HAL 回调函数体未修改；现有 `tests/check_refactor.py` 在进入本模块检查前即因已迁移的 `MX_USART3_UART_Init` 旧基线断言失败，已留给 `tests/` 回归基础设施模块处理。
- 资源结果：`Code=18170 → 17970`（-200 B），`ZI-data=2516 → 2444`（-72 B）；`RO-data=650`、`RW-data=12` 不变。
- 待办事项：
  - [ ] 在回归基础设施模块同步已迁移的 USART 初始化结构断言，恢复自动化检查入口。

#### Diff

```diff
-#include "gpio.h"

-volatile uint8_t serial_motion_active;
+static volatile uint8_t serial_motion_active;
 /* 其余 15 个 ISR 状态量同样仅增加 static，类型、顺序和地址传递均不变。 */
```
