### 模块：`MotorContrl/APPs/app_self_test.c` + `MotorContrl/Inc/app_self_test.h`

> 后续变更（2026-09-03）：该历史 X 轴自检模块已按用户要求删除；本文以下内容是删除前的扫描记录，不再代表当前工程状态。

- 扫描时间：2026-09-02 18:38:15 +08:00
- 当前条件编译配置：`LIMIT_GPIO_STATIC_TEST=0U`、`SERIAL_PROTOCOL_STAGE1_TEST=1U`，故表达式 `!LIMIT_GPIO_STATIC_TEST && !SERIAL_PROTOCOL_STAGE1_TEST` 为假，生产自检当前关闭。工程另启用 `USE_HAL_DRIVER`、`STM32F103xE`，未启用 `DEBUG`、`TEST`、`TRACE`、`LOG`、`DBG`。
- 静态扫描：
  - 全局/静态变量：无。
  - 公开函数：`AppSelfTest_Init`、`AppSelfTest_Process`；声明保留在 `app_self_test.h`，启用自检时由调度器调用。
  - 共享状态类型：`AppTmcState` 保存 SPI/GCONF/GSTAT/CHOPCONF 检查结果、延时 tick/pending、单步/多步相位与 MSCNT、连续运动结果。这些字段共同构成生产自检状态机及可观察测试结果，不因 `test` 命名删除。
  - 局部变量：无；所有跨轮询状态均必须持久化于 `AppTmcState`，设备句柄由调用方传入。
  - 宏：头文件保护 `MOTOR_APP_SELF_TEST_H`；模块使用现有自检开关、阶段值、延时、步数、电流和 TMC 寄存器宏，未新增业务宏。
  - 调试属性命名：未发现 `debug_`、`dbg_`、`tmp_`、打印缓冲或调试输出。文件末尾存在一整段已注释、且引用旧变量的辅助输出测试代码，已删除。
- 引用分析：
  - 两个公开函数仅由 `app_scheduler.c` 调用；周期处理原本已有自检条件保护，初始化调用和私有 `runtime.tmc` 状态分配现使用完全相同的条件。
  - 当前配置下 `runtime.tmc` 不再进入固件对象，初始化/轮询均不执行；不会改变 GPIO、SPI、定时器或 TMC 状态。
  - 自检开启配置下，SPI 版本检查、GCONF 写回读、GSTAT 清除等待、CHOPCONF/电流配置、驱动使能检查、单步、多步、定时器运动与 6 s 禁用顺序全部保留。
  - `gpio.h`、`tim.h` 和 `tmc5160.h` 在自检开启配置仍被使用，不能清理。
- 删除的变量/宏/函数：
  - 当前配置中的 `AppRuntime.tmc` 实例（条件排除）— 原因：生产自检关闭时无任何消费者；自检开启时仍完整保留。
  - 无永久删除的公开函数、类型字段或宏。
- 合并/简化的逻辑：
  - 将自检状态分配、初始化和周期处理统一置于同一现有条件下，消除关闭配置仍分配/初始化状态的配置不对称。
  - 删除文件末尾 15 行已注释的旧辅助输出翻转测试分支，并清除 10 行残留空白。
- 保留但标记为“待确认”的项：无。生产自检属于可重新启用功能，所有寄存器结果与状态字段保留。
- 修改后代码行数变化：`app_self_test.c` 272 → 251 行（-21），`app_self_test.h` 18 行不变；调度器增加 4 行条件保护，固件源码净减少 17 行。测试桩增加 2 行，仅在自检关闭模式跳过不存在的自检状态快照。
- 编译检查：通过。Keil Arm Compiler 6.24 全量构建 0 errors、0 warnings。
- 结构/行为检查：当前配置的冻结结构及三轴 case 49 通过。自检开启模式可正常编译；直接运行 case 48 至 7100 ms 后，旧/新固件共 138 条 `x_tmc5160_*` 状态输出完全一致。完整差分仍先停在既有初始化 HAL 轨迹差异处。
- 资源结果：相对协议模块完成后，`Code` 18222 → 18198（-24 B），`ZI-data` 2652 → 2516（-136 B）；`RO-data=650`、`RW-data=12` 不变。
- 时序/安全检查：未改动任一 `HAL_GetTick()` 差值、TMC 寄存器访问、STEP 高低相位、定时器启停、使能失败关闭或最终超时关闭逻辑。
- 待办事项：无。

#### Diff

```diff
 void AppSelfTest_Init(AppTmcState *tmc)
 {
+#if !LIMIT_GPIO_STATIC_TEST && !SERIAL_PROTOCOL_STAGE1_TEST
   /* 原初始化 */
+#else
+  (void)tmc;
+#endif
 }

 typedef struct {
   /* protocol/motion/aux/limits */
+#if !LIMIT_GPIO_STATIC_TEST && !SERIAL_PROTOCOL_STAGE1_TEST
   AppTmcState tmc;
+#endif
   const AppMotionIrq *irq;
 } AppRuntime;

+#if !LIMIT_GPIO_STATIC_TEST && !SERIAL_PROTOCOL_STAGE1_TEST
   AppSelfTest_Init(&runtime.tmc);
+#endif

-/* 已注释的 auxiliary_output_test_state 旧测试分支，共 15 行 */
```

注：这里按当前配置只消除未启用功能的运行时成本，不删除生产自检实现；切换 `SERIAL_PROTOCOL_STAGE1_TEST=0U` 后，原状态、初始化与轮询会同时恢复。
