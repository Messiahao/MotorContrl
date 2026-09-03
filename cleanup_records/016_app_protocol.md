### 模块：`MotorContrl/APPs/app_protocol.c` + `MotorContrl/Inc/app_protocol.h`

- 扫描时间：2026-09-02 18:32:01 +08:00
- 当前条件编译配置：工程启用 `USE_HAL_DRIVER`、`STM32F103xE`；未启用 `DEBUG`、`TEST`、`TRACE`、`LOG`、`DBG`。本模块仅有头文件保护和 C++ 链接包装，无业务条件分支。
- 静态扫描：
  - 全局变量：无。
  - 静态变量：`AppProtocol_Process` 内的只读测试命令响应帧 `response[14]`，每次调用复用，参与 UART 发送。
  - 公开函数：`AppProtocol_Init`、`AppProtocol_Process`；均由 `app_scheduler.c` 调用并保留头文件声明。
  - 局部变量：`byte`、`frame_ok`；分别保存当前 UART 字节和完整测试帧判定结果。
  - 宏：头文件保护 `MOTOR_APP_PROTOCOL_H`；帧长度、头尾、命令、字段索引和超时均来自 `config.h`。
  - 调试属性命名：协议状态沿用历史 `serial_test_` 前缀。逐项跨工程确认后，收帧缓冲和索引属于解析器工作状态；其余八个字段仅用于调试器观察且从未被业务读取。
- 引用分析：
  - `serial_test_rx_frame` 在收帧、测试帧校验和命令回调中读写；`serial_test_rx_index` 在同步双帧头、累计 14 字节和完整帧后复位时读写，均保留。
  - 已删除八个协议诊断字段：其中七个仅写不读；`serial_test_last_frame_ok` 唯一控制读取位于当前函数，已替换为局部 `frame_ok`，逻辑等价。
  - `SERIAL_BUILD_MARKER` 只有被删除字段的单一写入点，随之删除。
  - UART ORE 清除、逐字节读取、双 `0x55` 重同步、测试帧回包以及非测试帧命令分发调用顺序均保持不变。
  - 为清理共享协议诊断字段，删除了 `app_motion.c`、`app_aux_output.c`、`app_light.c` 和 `app_scheduler.c` 中仅写入这些字段的镜像赋值/计数；公开函数参数与接口未改变。
- 删除的变量/宏/函数：
  - `serial_test_build_marker` — 原因：初始化时写入后全工程从未读取，仅供调试器识别构建。
  - `serial_test_command_count` — 原因：仅在测试命令回包时递增，从未读取。
  - `serial_test_frame_error_count` — 原因：仅在各命令错误路径递增，从未读取，不参与错误响应。
  - `serial_test_rx_byte_count` — 原因：每收一字节递增，从未读取。
  - `serial_test_uart_error_count` — 原因：ORE 时仅递增，从未读取；ORE 清除操作保留。
  - `serial_test_last_rx_byte` — 原因：仅复制当前字节，从未读取。
  - `serial_test_last_frame_ok` — 原因：跨模块均仅写；协议解析器内唯一读取已局部化。
  - `serial_test_last_response_ok` — 原因：仅镜像各 UART 发送结果，从未读取；发送调用及业务模块自身结果状态保留。
  - `#define SERIAL_BUILD_MARKER` — 原因：删除构建标记字段后无引用。
- 合并/简化的逻辑：
  - 测试帧判定使用局部 `frame_ok`，消除 volatile 共享状态的一写一读。
  - 删除 UART 收字节、错误、命令和响应结果的无消费统计/镜像写入，保留所有硬件副作用。
  - `AppProtocol_Init` 从写入调试标记改为显式复位真实解析状态 `serial_test_rx_index`，公开初始化接口保持不变。
- 保留但标记为“待确认”的项：无。固定响应帧、接收缓冲、接收索引和回调均为协议必需。
- 修改后代码行数变化：固件源码合计 -32 行；其中 `app_protocol.c` 77 → 73 行，`app_protocol.h` 19 行不变，共享 `app_types.h` 183 → 175 行，`config.h` 133 → 132 行，其余四个应用文件合计减少 19 行。测试桩增加 11 行过滤已删除的纯诊断快照。
- 编译检查：通过。Keil Arm Compiler 6.24 全量构建 0 errors、0 warnings。
- 结构/行为检查：冻结结构检查通过；三轴专用 case 49 通过。完整历史差分已能正常编译，仍在既有 `default case 0` 初始化 HAL 轨迹差异处失败：旧 `events=19`，新 `events=36`。
- 资源结果：`Code` 18546 → 18222（-324 B），`ZI-data` 2676 → 2652（-24 B）；`RO-data=650`、`RW-data=12` 不变。
- 待办事项：无。

#### Diff

```diff
-#define SERIAL_BUILD_MARKER 0x20260826U

 typedef struct {
   uint8_t serial_test_rx_frame[SERIAL_TEST_FRAME_SIZE];
   uint8_t serial_test_rx_index;
-  volatile uint32_t serial_test_command_count;
-  volatile uint32_t serial_test_frame_error_count;
-  volatile uint32_t serial_test_rx_byte_count;
-  volatile uint32_t serial_test_uart_error_count;
-  volatile uint8_t serial_test_last_rx_byte;
-  volatile uint8_t serial_test_last_frame_ok;
-  volatile uint8_t serial_test_last_response_ok;
-  volatile uint32_t serial_test_build_marker;
 } AppProtocolState;

-protocol->serial_test_build_marker = SERIAL_BUILD_MARKER;
+protocol->serial_test_rx_index = 0U;
-/* 无消费的收字节/错误/命令计数、最近字节和跨模块帧/响应镜像写入 */
+/* 协议内帧判定改用局部 frame_ok；UART、GPIO、TMC 和命令回调调用均保留 */
```

注：`config.h`、`app_types.h` 和 `app_motion.c` 还包含本轮开始前已有的三轴改动；本记录仅统计上述协议诊断清理。`tests/check_refactor.py` 仅停止快照已删除字段，仍比较 UART/HAL 轨迹、收帧缓冲、索引和全部业务状态。
