### 模块：`MotorContrl/APPs/app_light.c` + `MotorContrl/Inc/app_light.h`

- 扫描时间：2026-09-02 17:30:28 +08:00
- 当前条件编译配置：`USE_HAL_DRIVER`、`STM32F103xE`；未定义 `DEBUG`、`TEST`、`TRACE`、`LOG`、`DBG`。模块自身仅有头文件保护和 C++ 链接包装。
- 静态扫描：
  - 全局变量、文件静态变量：无。
  - 文件静态函数：`Serial_Light_SendResponse`；复制请求有效载荷并生成成功响应。
  - 公开函数：`AppLight_Init`、`AppLight_Process`；均在头文件声明，后者由调度器调用。
  - 局部变量：14 字节 `response`；`action`、`protocol_channel`、`frame_ok`、`data_ok`、`action_ok`、`channel_ok`。原一次性 `code` 已删除。
  - 宏：头文件保护 `MOTOR_APP_LIGHT_H`；通道映射宏 `APP_LIGHT_CHANNEL_1/2/3/4`。
  - 调试属性命名：无调试变量、测试函数、打印缓冲或无用计数器；协议错误计数为可观察业务状态。
- 引用分析：
  - 空初始化函数由调度器调用，有意保持 MCP4728 上电恢复值不变，不能改成主动清零。
  - 处理函数校验帧尾、未使用数据字节、动作及 1～4 通道；合法后调用 `BspMcp4728_Write`，成功才发送响应。
  - 通道 1、4 用于边界及主机测试；通道 2、3 当前没有独立代码引用，但四个公开宏共同定义协议至 DAC A～D/CN10-1～4 的映射，按接口兼容要求保留。
  - 响应帧数组由阻塞 UART 发送读取，不存在只写不读或悬空引用。
- 删除的变量/宏/函数：
  - `code`（`AppLight_Process` 局部变量）— 原因：只接收一次条件表达式结果并立即传给 DAC 写接口。
- 合并/简化的逻辑：
  - 将输入无效和 MCP4728 写失败合并到同一错误出口；依靠 `||` 短路保证输入无效时不执行 DAC 写入。
  - 将开/关对应的满量程/零码值直接传入唯一调用处。
- 保留但标记为“待确认”的项：无。
- 修改后代码行数变化：-6 行（`app_light.c` 原 70 行 → 现 64 行；`app_light.h` 原 22 行 → 现 22 行）。
- 编译检查：通过。Keil Arm Compiler 6.24 重新编译 `app_light.c`，构建 0 errors、0 warnings。
- 结构/行为检查：灯板专用主机 case 18 通过，覆盖通道 4 满量程、通道 1 关闭及非法动作不产生 I²C 写；冻结结构检查通过；标准脚本仍在既有 `default case 0` 初始化轨迹差异处提前失败。
- 资源结果：链接映像保持 `Code=18634`、`RO-data=650`、`RW-data=12`、`ZI-data=2700`。
- 硬件/协议检查：保持命令 `0x0500`、动作 `0x01/0x02`、协议通道到 MCP4728 A～D 的减一映射、开灯 4095/关灯 0、成功后原帧字段回包及失败不回包行为。
- 待办事项：
  - [ ] 改板后继续完成 MCP4728 ACK、电平、四路映射和输出电压实机验证。

#### Diff

```diff
-  uint16_t code;
-  if (invalid_input) { error; return; }
-  code = on ? BSP_MCP4728_CODE_FULL_SCALE : BSP_MCP4728_CODE_OFF;
-  if (BspMcp4728_Write(channel, code) == 0U) { error; return; }
+  if (invalid_input ||
+      (BspMcp4728_Write(channel,
+          on ? BSP_MCP4728_CODE_FULL_SCALE : BSP_MCP4728_CODE_OFF) == 0U))
+  { error; return; }
```

后续关联记录：协议层处理时删除了本模块对三个纯诊断协议字段的写入，DAC 写入和 UART 响应保持不变，详见 `cleanup_records/016_app_protocol.md`。

后续关联记录：调度器处理时移除了对空实现 `AppLight_Init` 的内部调用，公开接口保留，详见 `cleanup_records/018_app_scheduler.md`。
