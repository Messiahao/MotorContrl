### 模块：`MotorContrl/APPs/app_motion.c` + `MotorContrl/Inc/app_motion.h`

- 扫描时间：2026-09-02 18:22:33 +08:00
- 当前条件编译配置：工程启用 `USE_HAL_DRIVER`、`STM32F103xE`；未启用 `DEBUG`、`TEST`、`TRACE`、`LOG`、`DBG`。`app_motion` 本身仅含 C/C++ 头文件保护条件。当前 `Z_LIMIT_PROTECTION_ENABLED=0U`，Z 轴限位输入仍采样，但不会触发运动阻止或停机。
- 静态扫描：
  - 全局/静态变量：无。模块运行状态全部由调用方传入的 `AppMotionState`、`AppMotionIrq` 和轴配置表持有。
  - 文件内静态函数：`Serial_Motion_SendResponse`、`Serial_Motion_SendStatusResponse`、`Serial_Motion_FindAxis`、`Serial_Motion_GetAxisState`、`Serial_Motion_PrepareAndStart`。
  - 公开函数：`AppMotion_Init`、`AppMotion_ProcessStart`、`AppMotion_ProcessStop`、`AppMotion_ProcessStatus`、`AppMotion_Process`。
  - 公开类型：`AppMotionAxisConfig`；集中保存协议轴号、TMC 句柄、DIR/STEP GPIO、限位掩码与电流参数。
  - 宏：仅头文件保护宏 `MOTOR_APP_MOTION_H`；业务常量均来自统一配置或相关驱动头文件。
  - 调试属性命名：未发现 `debug_`、`dbg_`、`tmp_`、打印缓冲或调试条件分支。`serial_test_*` 是现有协议统计/响应可观测状态，`*_test_done` 是 TMC 启动自检结果，均参与对外状态，不属于可删除调试残留。
- 引用分析：
  - 五个公开函数均由 `app_scheduler.c` 调用；`AppMotionAxisConfig` 由调度器的 X/Y/Z 配置表实例化，属于有效公开接口。
  - 五个静态函数均在本文件被调用；唯一调用的 `Serial_Motion_PrepareAndStart` 包含启动前、中、后的重复限位检查、SPI 配置校验、GPIO 模式切换和定时器启动，是时序/安全边界，不内联。
  - 响应缓冲均传入 UART 发送，状态、错误码、计数器、MSCNT 前后值均被协议响应或后续状态机读取；未发现只写不读的模块私有符号。
  - 当前工作树在本轮开始前已有三轴通用化改动；本次完整保留，未将其计入精简结果。
- 删除的变量/宏/函数：无。
- 合并/简化的逻辑：无。停止、限位停止、启动失败与正常完成分支虽有相似赋值，但响应状态、错误码及 ISR 标志清理顺序不同；合并会扩大时序关键改动范围。
- 保留但标记为“待确认”的项：无。
- 修改后代码行数变化：0 行（`app_motion.c` 589 行、`app_motion.h` 41 行；合计 630 行保持不变）。
- 编译检查：通过。Keil Arm Compiler 6.24 全量构建 0 errors、0 warnings。
- 结构/行为检查：`python tests/check_refactor.py --three-axis-only` 通过冻结结构检查及 X/Y/Z 轴选择、定时器路由、完成、停止、限位隔离验证。
- 资源结果：无源代码修改，沿用当前构建结果 `Code=18546`、`RO-data=650`、`RW-data=12`、`ZI-data=2676`，无增减。
- 待办事项：
  - [ ] Z 轴限位开关补齐并完成实机测试后，再评估将 `Z_LIMIT_PROTECTION_ENABLED` 恢复为 `1U`；当前不得宣称 Z 轴保护已启用。

#### Diff

```diff
# 本模块经静态扫描、全工程引用分析、三轴回归及 Keil 构建后无需修改。
```

注：当前 `git diff` 中 `app_motion.c/.h` 的三轴通用化内容是本轮开始前已有修改，本轮未改写或回退。

后续关联记录：协议层处理时删除了本模块对三个纯诊断协议字段的镜像写入，运动状态机自身字段、响应和时序不变，详见 `cleanup_records/016_app_protocol.md`。
