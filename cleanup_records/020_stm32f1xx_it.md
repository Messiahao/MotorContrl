### 模块：`MotorContrl/Src/stm32f1xx_it.c` + `MotorContrl/Inc/stm32f1xx_it.h`

- 扫描时间：2026-09-04 13:40:41 +08:00
- 当前条件编译配置：Keil AC6.24，定义 `USE_HAL_DRIVER`、`STM32F103xE`；未定义 `DEBUG`、`TEST`、`TRACE`、`LOG`、`DBG`。模块仅含头文件保护和 C++ 链接包装。
- 静态扫描：
  - 全局/静态变量、私有函数、业务宏：无。
  - 处理器异常函数：`NMI_Handler`、`HardFault_Handler`、`MemManage_Handler`、`BusFault_Handler`、`UsageFault_Handler`、`SVC_Handler`、`DebugMon_Handler`、`PendSV_Handler`、`SysTick_Handler`。
  - 外设中断函数：`EXTI9_5_IRQHandler`、`EXTI15_10_IRQHandler`、`TIM2_IRQHandler`、`TIM3_IRQHandler`、`TIM4_IRQHandler`。
  - include：`main.h` 提供限位引脚/HAL 接口，`stm32f1xx_it.h` 提供声明，`tim.h` 提供三个定时器句柄，均有实际用途。
  - 调试属性命名：`DebugMon_Handler` 是 Cortex-M3 标准异常入口，不是调试辅助函数；无调试变量、打印或计数器。
- 引用分析：
  - 14 个函数均由 `startup_stm32f103xe.s` 中断向量表引用，头文件声明与函数定义一一对应。
  - `SysTick_Handler` 必须调用 `HAL_IncTick`，为 HAL 超时和软件节拍提供时间基准。
  - EXTI9_5 处理 PC6/PC7/PC8/PC9，EXTI15_10 处理 PA10/PA11/PA12、PB14/PB15，与九路限位原理图和 GPIO NVIC 配置一致。
  - TIM2/3/4 分别把中断交给对应 HAL 定时器句柄，随后进入三轴运动回调；不能合并或改变调用顺序。
  - 空的 SVC、DebugMon、PendSV 当前执行后返回；删除定义会落入启动文件的弱默认死循环，属于行为改变，故保留。
- 删除的变量/宏/函数：无。
- 合并/简化的逻辑：无。ISR 必须短小且入口独立，合并封装会增加跳转并破坏向量接口。
- 保留但标记为“待确认”的项：无。
- 修改后代码行数变化：0 行（`.c` 254 行、`.h` 71 行，合计 325 行不变）。
- 编译检查：通过。当前源码已由 Keil Arm Compiler 6.24 完整重建，0 errors、0 warnings；本模块未修改。
- 结构/行为检查：冻结结构检查通过；向量表、九路限位映射、EXTI NVIC 和 TIM2/3/4 句柄交叉引用完整。
- 资源结果：无源码修改，保持 `Code=18170`、`RO-data=650`、`RW-data=12`、`ZI-data=2516`。
- 待办事项：无。

#### Diff

```diff
# 中断入口、向量绑定和 HAL 转发均为有效代码，本模块无需修改。
```
