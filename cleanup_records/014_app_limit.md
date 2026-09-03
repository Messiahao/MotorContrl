### 模块：`MotorContrl/APPs/app_limit.c` + `MotorContrl/Inc/app_limit.h`

- 扫描时间：2026-09-02 17:34:04 +08:00
- 当前条件编译配置：`USE_HAL_DRIVER`、`STM32F103xE`；未定义 `DEBUG`、`TEST`、`TRACE`、`LOG`、`DBG`。当前 `Z_LIMIT_PROTECTION_ENABLED=0U`：Z 轴 PA10/PA11/PA12 仍采样和映射，但 Z 轴保护掩码为 0。
- 静态扫描：
  - ISR/任务共享全局变量：`g_limit_irq_active_mask`、`g_limit_irq_event_mask`；均为 `volatile` 且保留头文件 `extern` 声明。
  - 已删除调试/只写全局变量：`g_limit_debug`、`g_limit_debug_sample_count`、`g_limit_debug_gpio_c_idr`、`g_limit_debug_pc6_direct`、`g_limit_irq_event_count`。
  - 公开函数：`AppLimit_Init`、`AppLimit_Process`、`AppLimit_OnExti`、`AppLimit_ConsumeInterruptMask`。
  - 局部变量：`active_mask`、`limit_bit`、`axis_limit_mask`、`event_mask`、`primask`；分别用于稳定快照、引脚映射、当前轴过滤和临界区消费。
  - 宏：头文件保护 `MOTOR_APP_LIMIT_H`；本模块未定义业务宏，限位电平、位编号和 X/Y/Z 保护掩码来自统一配置与 GPIO 驱动。
  - 调试属性命名：四个 `g_limit_debug*` 已全部删除；未发现其他调试函数或缓冲。
- 引用分析：
  - `g_limit_irq_active_mask` 由 EXTI 更新、前台采样读取；`g_limit_irq_event_mask` 由 EXTI 置位，并由前台在保存 PRIMASK 的临界区中读取后清零，均参与控制流。
  - `g_limit_irq_event_count` 仅在初始化时清零、有效沿时递增，工程内没有读取；四个 debug 快照仅写入并通过头文件暴露，不参与限位判断、急停或协议响应。
  - `AppLimit_Init/Process/ConsumeInterruptMask` 由调度器调用；`AppLimit_OnExti` 由 `HAL_GPIO_EXTI_Callback` 调用。
  - 当前工作树原有的 `active_axis` 参数、`BspGpio_LimitMaskForAxis` 与 `BspTim_WriteAxisEmergencyStop` 改动属于用户已有修改，本次完整保留。
  - 九路限位电平字段和活动掩码均属于可观察运行状态，不按调试变量删除。
- 删除的变量/宏/函数：
  - `g_limit_debug` — 原因：每次采样复制完整状态，仅供调试观察，工程内无读取。
  - `g_limit_debug_sample_count` — 原因：仅递增、从未读取。
  - `g_limit_debug_gpio_c_idr` — 原因：仅保存 PC 端口原始 IDR 调试快照，控制逻辑不使用。
  - `g_limit_debug_pc6_direct` — 原因：仅保存由上述快照推导的 PC6 调试电平，控制逻辑不使用。
  - `g_limit_irq_event_count` — 原因：仅清零和递增、从未读取，不影响事件掩码消费。
- 合并/简化的逻辑：无。ISR 位图更新、按轴急停和 PRIMASK 临界区保持原结构；只删除无行为的观测写入。
- 保留但标记为“待确认”的项：无。两个剩余全局量虽然仅在本模块使用，但属于公开且跨 ISR/前台共享的状态；按接口兼容要求不改为 `static`。
- 修改后代码行数变化：-18 行（`app_limit.c` 本轮处理前 102 行 → 现 90 行；`app_limit.h` 本轮处理前 28 行 → 现 22 行）。
- 编译检查：通过。Keil Arm Compiler 6.24 重新编译 `app_limit.c`、受头文件影响的调度器和入口，构建 0 errors、0 warnings。
- 结构/行为检查：冻结结构检查通过；三轴专用 case 49 通过，覆盖 X/Y/Z 轴选择、Y 轴限位急停、跨轴隔离、Z 轴保护关闭和停止路径；标准差分脚本仍在既有 `default case 0` 初始化轨迹差异处提前失败。
- 资源结果：`Code` 从 18634 降至 18546（-88 B），`ZI-data` 从 2700 降至 2676（-24 B）；`RO-data=650`、`RW-data=12` 不变。
- 中断/安全检查：保持有效沿置位活动/事件掩码、释放沿清活动位、仅当前运动轴匹配时急停、保持 ENN 低电平维持抱持力，以及前台原子消费事件掩码的行为。删除后 ISR 少一次无用的 32 位计数递增。
- 待办事项：
  - [ ] Z 轴限位开关补齐并实测后，再将 `Z_LIMIT_PROTECTION_ENABLED` 恢复为 `1U`；当前不得宣称 Z 轴保护已启用。

#### Diff

```diff
-volatile AppLimitState g_limit_debug;
-volatile uint32_t g_limit_debug_sample_count;
-volatile uint32_t g_limit_debug_gpio_c_idr;
-volatile uint8_t g_limit_debug_pc6_direct;
-volatile uint32_t g_limit_irq_event_count;
-/* 对应初始化、快照写入、直接 IDR 读取及 ISR 递增 */
-/* app_limit.h 中上述五个 extern 声明 */
```

注：当前 `git diff` 还包含本轮开始前已经存在的按轴限位修改；上述 Diff 仅列出本轮精简内容。
