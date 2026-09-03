### 模块：`MotorContrl/APPs/app_scheduler.c` + `MotorContrl/Inc/app_scheduler.h`

- 扫描时间：2026-09-02 18:45:32 +08:00
- 当前条件编译配置：工程启用 `USE_HAL_DRIVER`、`STM32F103xE`；未启用 `DEBUG`、`TEST`、`TRACE`、`LOG`、`DBG`。`LIMIT_GPIO_STATIC_TEST=0U`、`SERIAL_PROTOCOL_STAGE1_TEST=1U`，生产自检当前关闭；`Z_LIMIT_PROTECTION_ENABLED=0U`，Z 轴配置表限位掩码为 0。
- 静态扫描：
  - 文件私有类型：`AppRuntime`，集中持有协议、运动、辅助输出、限位、条件自检状态和 ISR 桥接指针。
  - 文件静态变量：`runtime`；由初始化、帧回调和固定轮询共同读写。
  - 文件静态常量：`motion_axes[3]`；保存 X/Y/Z 的协议轴号、TMC 句柄、DIR/STEP GPIO、限位掩码和电流参数。
  - 文件静态函数：`Scheduler_ProcessFrame`；作为 `AppProtocol_Process` 的命令分发回调。
  - 公开函数：`AppScheduler_Init`、`AppScheduler_Process`、`AppScheduler_Run`；分别由 `main.c`、主循环及主机回归使用，全部保留头文件声明。
  - 宏：头文件保护 `MOTOR_APP_SCHEDULER_H`；模块未定义业务宏，只消费统一配置和板级引脚宏。
  - 调试属性命名：未发现 `debug_`、`dbg_`、`tmp_`、打印缓冲或无消费计数器；`AppSelfTest_*` 是可重新启用的生产自检接口。
- 引用分析：
  - `runtime` 的协议、运动、辅助输出、限位和 IRQ 字段均参与初始化或轮询；`tmc` 字段仅在自检启用配置进入对象。
  - `motion_axes` 被运动初始化、启动、停止、前台处理和自检共同引用；三轴条目及 Z 轴关闭保护的掩码均保留。
  - 三个公开函数和私有帧回调均有调用；不存在未调用静态函数。
  - `AppAuxOutput_Init`、`BspMcp4728_Init`、`AppLight_Init` 的实现均为空，工程内没有必须依赖调用动作；移除调度器调用但保留公开 API。
  - `mcp4728.h` 在删除空初始化调用后无剩余符号引用，已清理；其他 13 个 include 均有直接类型、函数或引脚宏依赖。
- 删除的变量/宏/函数：
  - 局部变量 `limit_event_mask` — 原因：只接收一次 `AppLimit_ConsumeInterruptMask()` 结果后立即传参，可安全直接传递。
  - 无删除的全局/静态变量、宏或函数；公开接口完全保留。
- 合并/简化的逻辑：
  - 命令码和子命令各读取一次；将三个重复的运动命令判断归入单一运动分支，并提取继电器/抱闸的共同默认子命令条件。
  - 删除三个空初始化函数的内部调用，同时移除无用的 `mcp4728.h` include；所有真实硬件初始化顺序不变。
  - 将限位事件掩码消费直接传入 `AppMotion_Process`，保持“采样 → 原子消费 → 运动处理”的顺序。
- 保留但标记为“待确认”的项：无。
- 修改后代码行数变化：-6 行（`app_scheduler.c` 本轮处理前 153 行 → 现 147 行；`app_scheduler.h` 18 行不变）。
- 编译检查：通过。Keil Arm Compiler 6.24 全量构建 0 errors、0 warnings。
- 结构/行为检查：冻结结构及三轴 case 49 通过；测试帧、非法运动帧、继电器/抱闸/灯光和 UART 发送失败用例 1、2、18、19 均正常退出。完整历史差分仍停在既有 `default case 0` 初始化轨迹差异：旧 `661db29559361e9d/events=19`，新 `a107d59189a9ab12/events=35`。
- 资源结果：`Code` 18198 → 18170（-28 B）；`RO-data=650`、`RW-data=12`、`ZI-data=2516` 不变。
- 时序/安全检查：GPIO、USART2、I2C、SPI、TIM2/3/4、USART3、LED、运动和限位初始化的相对顺序不变；前台 LED、协议、限位、运动、自检轮询顺序不变；未触碰 ISR、寄存器映射或定时参数。
- 待办事项：无。

#### Diff

```diff
-#include "mcp4728.h"

+uint8_t command = frame[SERIAL_FRAME_COMMAND_INDEX];
+uint8_t subcommand = frame[SERIAL_FRAME_SUBCOMMAND_INDEX];
-/* 重复读取 command/subcommand 的平铺分支 */
+/* 单一 motion 分支；relay/brake 共用默认子命令条件 */

-AppAuxOutput_Init();
-BspMcp4728_Init();
-AppLight_Init();

-uint16_t limit_event_mask;
-limit_event_mask = AppLimit_ConsumeInterruptMask();
-AppMotion_Process(..., limit_event_mask);
+AppMotion_Process(..., AppLimit_ConsumeInterruptMask());
```

注：当前 `git diff` 还包含本轮开始前已有的三轴配置表、协议诊断清理和自检条件分配改动；分别见 `015`～`017` 记录，本记录只统计上述调度器精简。
