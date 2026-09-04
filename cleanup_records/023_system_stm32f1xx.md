### 模块：`MotorContrl/Src/system_stm32f1xx.c`

- 扫描时间：2026-09-04 13:49:24 +08:00
- 当前条件编译配置：Keil AC6.24，定义 `USE_HAL_DRIVER`、`STM32F103xE`；未定义 `DATA_IN_ExtSRAM`、`USER_VECT_TAB_ADDRESS`、`VECT_TAB_SRAM`，因此外部 SRAM 初始化和向量表重定位分支不编译。
- 静态扫描：
  - 全局变量/表：`SystemCoreClock`、`AHBPrescTable[16]`、`APBPrescTable[8]`。
  - 公开函数：`SystemInit`、`SystemCoreClockUpdate`。
  - 条件私有函数：`SystemInit_ExtMemCtl`，仅在 `DATA_IN_ExtSRAM` 启用时存在；当前不生成代码。
  - 本地变量：`SystemCoreClockUpdate` 内的 `tmp`、`pllmull`、`pllsource`；其他 STM32F1 型号专用预分频变量均被条件编译排除。
  - 宏：`HSE_VALUE`、`HSI_VALUE` 仅在上层未定义时提供默认值；向量表基址/偏移宏仅在重定位配置下定义。
  - include：仅 `stm32f1xx.h`，提供器件寄存器、CMSIS 接口和系统配置，不能删除。
  - 调试属性：无调试变量、测试函数、打印缓存或 DEBUG 分支。
- 引用分析：
  - `SystemInit` 由 `startup_stm32f103xe.s` 在进入 C 运行库/`main` 前直接导入调用，不能删除或内联。
  - `SystemCoreClock` 被 HAL SysTick、RCC、I2C、SPI 等有效路径读取，并由时钟配置更新。
  - `AHBPrescTable`、`APBPrescTable` 由 HAL RCC 的 HCLK/PCLK 计算使用，且由 CMSIS 头文件以 `extern` 暴露。
  - `SystemCoreClockUpdate` 当前没有运行时调用，链接器已自动移除其 92 B 代码；它是 CMSIS 标准公开接口，删除源码不会进一步减小固件，却会破坏外部接口。
  - 外部 SRAM、向量重定位及其他 STM32F1 型号分支当前均为零二进制成本；该文件与 CMSIS 官方模板一致，并被回归清单按原文冻结。
- 删除的变量/宏/函数：无。
- 合并/简化的逻辑：无。修改该启动/时钟模板会增加时钟与启动回归风险，且不会带来当前固件收益。
- 保留但标记为“待确认”的项：无。
- 修改后代码行数变化：0 行（406 行不变）。
- 编译检查：通过。前一模块触发本文件重编译，Keil Arm Compiler 6.24 为 0 errors、0 warnings。
- 结构/行为检查：启动文件对 `SystemInit` 的引用、HAL 对三个时钟对象的引用和 CMSIS 公开声明均完整；源码与 vendor 模板一致。
- 资源结果：无源码修改，保持 `Code=17970`、`RO-data=650`、`RW-data=12`、`ZI-data=2444`。
- 待办事项：无。

#### Diff

```diff
# CMSIS 启动/时钟接口均有效，未产生源码修改。
```
