### 模块：`MotorContrl/Drivers/Board/usart.c` + `MotorContrl/Inc/usart.h`

- 扫描时间：2026-09-02 13:40:10 +08:00
- 当前条件编译配置：Keil 目标定义 `USE_HAL_DRIVER`、`STM32F103xE`；未定义 `DEBUG`、`TEST`、`TRACE`、`LOG`、`DBG`。`USE_HAL_UART_REGISTER_CALLBACKS=0U`，因此 HAL 直接调用传统 MSP 回调。头文件的 `__cplusplus` 包装因包含公开函数声明而保留。
- 静态扫描：
  - 文件静态全局变量：`huart2`、`huart3`；分别保存 USART2、USART3 的 HAL 句柄，无 `extern` 暴露。
  - 文件静态函数：`MX_USART2_UART_Init`、`MX_USART3_UART_Init`。
  - HAL 回调函数：`HAL_UART_MspInit`、`HAL_UART_MspDeInit`。
  - 公开初始化函数：`BspUsart2_Init`、`BspUsart3_Init`。
  - 公开轮询接收接口：`BspUsart_ReadOverrun`、`BspUsart_WriteClearOverrun`、`BspUsart_ReadAvailable`、`BspUsart_ReadByte`。
  - 公开发送接口：`BspUsart_Write`。
  - 局部变量：`HAL_UART_MspInit` 中的 `GPIO_InitStruct`；其字段在 USART2/USART3 分支内赋值后传给 `HAL_GPIO_Init`，不存在只写不读。
  - 宏：头文件保护宏 `__USART_H__`，配置宏 `BSP_USART_BAUD_RATE`、`BSP_USART_BYTE_MASK`；配置宏均被初始化或接收路径使用。
  - 调试属性命名：无 `DEBUG_`、`dbg_`、`debug_`、`test_`、`tmp_`、打印缓冲或调试计数器。
- 引用分析：
  - `BspUsart2_Init`、`BspUsart3_Init` 均由 `AppScheduler_Init` 调用，并保持原启动顺序；两个 MX 初始化函数分别是其唯一初始化入口。
  - `huart2` 的配置写入随后由 `HAL_UART_Init` 读取；USART2 对应 PA2/PA3、MAX3232/CN6 预留扩展口，虽然当前不收发数据，但初始化路径仍在运行。
  - `huart3` 同时由初始化、状态标志读取、DR 数据读取及 `HAL_UART_Transmit` 使用；USART3 是当前上位机接口。
  - 四个轮询接收接口均由 `app_protocol.c` 使用；发送接口由协议、灯光、运动和辅助输出模块使用。
  - `HAL_UART_MspInit` 由 `HAL_UART_Init` 在当前回调配置下直接调用；`HAL_UART_MspDeInit` 是 `HAL_UART_DeInit` 的框架入口，不能按“工程内无显式调用”删除。
  - `tests/check_refactor.py` 对两个 MX 初始化函数及两个 MSP 函数执行 token 级冻结比较，并检查初始化顺序和公开 USART 包装行为。
  - 未发现仅写不读、仅读不写且非 HAL/寄存器对象的变量。
- 删除的变量/宏/函数：无。
- 合并/简化的逻辑：无。USART2/USART3 初始化虽结构相似，但实例、GPIO、重映射和硬件用途不同，且初始化/MSP 函数属于冻结结构；简单公开包装承担板级边界，内联会改变接口或测试替换点。
- 保留但标记为“待确认”的项：无。`BSP_USART_BYTE_MASK` 虽可从 8 位强制转换推导，但它是已公开且实际使用的寄存器掩码，按接口兼容要求保留。
- 修改后代码行数变化：0 行（`usart.c` 原 231 行 → 现 231 行；`usart.h` 原 61 行 → 现 61 行）。
- 编译检查：通过。Keil Arm Compiler 6.24 构建 0 errors、0 warnings。
- 结构/行为检查：冻结结构检查通过；`tests/check_refactor.py` 随后仍在本轮处理前已存在的 `default case 0` 初始化轨迹差异处失败（旧 `661db29559361e9d/19`，新 `ef9af7514dad37d3/36`），本模块未产生新差异。
- 资源结果：链接映像保持 `Code=18674`、`RO-data=650`、`RW-data=12`、`ZI-data=2700`。
- 硬件/协议检查：保持 115200 baud、8 数据位、1 停止位、无校验、无硬件流控、16 倍过采样；保持 USART3 PC10/PC11 部分重映射至 MAX3232/CN4，USART2 PA2/PA3 至 MAX3232/CN6；未改变轮询标志、过载清除、DR 读取或发送超时行为。
- 待办事项：无。

#### Diff

```diff
# 无源码改动
```
