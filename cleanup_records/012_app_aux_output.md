### 模块：`MotorContrl/APPs/app_aux_output.c` + `MotorContrl/Inc/app_aux_output.h`

- 扫描时间：2026-09-02 17:26:33 +08:00
- 当前条件编译配置：`USE_HAL_DRIVER`、`STM32F103xE`；未定义 `DEBUG`、`TEST`、`TRACE`、`LOG`、`DBG`。模块自身仅有头文件保护和 C++ 链接包装。
- 静态扫描：
  - 全局变量、文件静态变量：无。
  - 文件静态函数：`Serial_Aux_SendResponse`；由继电器和抱闸成功路径复用。
  - 公开函数：`AppAuxOutput_Init`、`AppAuxOutput_Process`；均在头文件声明，后者由调度器调用。
  - 局部变量：14 字节 `response`；命令、动作、帧尾、数据、动作合法性和错误码局部值；均有读取路径。
  - 宏：头文件保护 `MOTOR_APP_AUX_OUTPUT_H`；本模块未定义业务宏，使用的协议索引、命令、动作、错误码及超时均来自统一配置头。
  - 调试属性命名：无调试变量、测试函数、打印缓冲或未使用计数器；`serial_*_command_count` 是协议可观察状态，不是调试计数器。
- 引用分析：
  - 初始化函数为空但由调度器调用，用于明确 GPIO 初始化已经建立默认关闭电平；属于公开生命周期接口。
  - 处理函数只从调度器的继电器/抱闸默认子命令分发路径进入；继电器与抱闸分别维护独立计数、状态、错误码和响应结果。
  - `Serial_Aux_SendResponse` 在两路成功路径调用；栈上响应帧随阻塞 UART 发送完成后释放，不存在悬空引用。
  - PB12/CN1 与 PB13/CN5 均为高电平使能；原 GPIO 写入极性和上电默认低电平保持不变。
  - 未发现只写不读、仅读不写或仅调试使用的符号。
- 删除的变量/宏/函数：无。
- 合并/简化的逻辑：
  - 将继电器、抱闸分支内完全重复的动作合法性判断和“帧错误 → 数据错误 → 动作错误 → 无错误”优先级提升到分支前计算一次。
  - 两个分支继续独立写入各自的状态字段、GPIO 和响应结果，未引入字段指针表或新的抽象层。
- 保留但标记为“待确认”的项：无。
- 修改后代码行数变化：-14 行（`app_aux_output.c` 原 118 行 → 现 104 行；`app_aux_output.h` 原 17 行 → 现 17 行）。
- 编译检查：通过。Keil Arm Compiler 6.24 重新编译 `app_aux_output.c`，构建 0 errors、0 warnings。
- 结构/行为检查：对帧合法/非法、数据合法/非法和全部 256 个动作码的 1024 种组合穷举比较，旧/新 `last_frame_ok` 与错误码零差异；冻结结构检查通过；标准脚本随后仍在既有 `default case 0` 初始化轨迹差异处失败。
- 资源结果：`Code` 从 18674 降至 18634（-40 B）；`RO-data=650`、`RW-data=12`、`ZI-data=2700` 不变。
- 硬件/协议检查：保持继电器命令 `0x03`、抱闸命令 `0x04`、动作 `0x01/0x02`、帧尾/零数据校验、错误优先级、GPIO 极性及成功后回包顺序。
- 待办事项：无。

#### Diff

```diff
+  aux_action_ok = (aux_action_code == SERIAL_AUX_ACTION_ON) ||
+                  (aux_action_code == SERIAL_AUX_ACTION_OFF);
+  /* 共享计算 aux_error_code：FRAME > DATA > ACTION > NONE。 */
   if (aux_command_code == SERIAL_COMMAND_RELAY)
   {
-    /* 分支内重复的动作校验和错误码 if/else */
+    aux->serial_relay_last_frame_ok = aux_frame_ok && aux_data_ok && aux_action_ok;
+    aux->serial_relay_error_code = aux_error_code;
   }
   else
   {
-    /* 分支内重复的动作校验和错误码 if/else */
+    aux->serial_brake_last_frame_ok = aux_frame_ok && aux_data_ok && aux_action_ok;
+    aux->serial_brake_error_code = aux_error_code;
   }
```

后续关联记录：协议层处理时删除了本模块对三个纯诊断协议字段的镜像写入，业务输出和本模块状态未变，详见 `cleanup_records/016_app_protocol.md`。

后续关联记录：调度器处理时移除了对空实现 `AppAuxOutput_Init` 的内部调用，公开接口保留，详见 `cleanup_records/018_app_scheduler.md`。
