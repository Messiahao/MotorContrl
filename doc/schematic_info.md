# MotorContrl 硬件原理图信息

## 使用范围与依据

- **唯一硬件引脚依据**：`SCH_Schematic1_2026-07-21.pdf`（单页原理图）。
- **控制器**：STM32F103RCT6，3.3 V 逻辑。
- 本文件只记录原理图可确认的连接关系。未在图中标明的定时器通道、GPIO 上下拉配置、EXTI 触发边沿、SPI 时钟频率和 TMC5160 寄存器参数，均须由固件设计和实机验证决定。
- 后续固件修改应先以本文件核对引脚。若实际硬件或更新版原理图与此不同，应先确认，再同步更新本文件。

## 板级功能概览

这是一个 24 V 供电的三轴两相步进电机控制板。每个轴使用一个 TMC5160A 和 8 颗外置 MOSFET 组成两相功率级；板上还提供 9 路隔离限位、两路 RS232、一路 5 V 继电器高边输出、一路 24 V Z 轴抱闸高边输出，以及四路灯板模拟控制输出。

## 电源与调试

| 项目 | 硬件连接 / 说明 |
| --- | --- |
| 24 V 输入 | CN2；F1 `0466001.NRHF` 保险丝与 D2 `SMBJ33A` TVS 保护。 |
| 5 V | U4 `TPS54331DDAR` 将 24 V 降压为 `+5V`。 |
| 3.3 V | U6 `TLV75733PDBVR` 将 +5V LDO 为 `+3V3`。 |
| 模拟/通信 5 V | +5V 经 L2 磁珠滤波形成 `+5Vc`，供 TLV9062 与 MAX3232。此网络**不是隔离电源**。 |
| 主时钟 | X1：8 MHz 晶振，接 PD0/OSC_IN、PD1/OSC_OUT。 |
| 复位 | NRST：R8 10 kΩ 上拉至 +3V3；SW1 按下接地；C29 100 nF。 |
| 启动配置 | BOOT0 通过 R11 10 kΩ 下拉至 GND。 |
| SWD | CN3：+3V3、PA13/SWDIO、PA14/SWCLK、NRST、GND。 |

## MCU 引脚总表

| GPIO | 原理图网络名 | 方向 / 外设用途 | 连接对象 |
| --- | --- | --- | --- |
| PA0 | XDIR | 输出 | X 轴 U18 `REFR_DIR`。 |
| PA1 | XSTEP | 输出 | X 轴 U18 `REFL_STEP`。 |
| PA2 | TX2 | USART2_TX | MAX3232 U5 第二路发送输入，外部接口 CN6。 |
| PA3 | RX2 | USART2_RX | MAX3232 U5 第二路接收输出，外部接口 CN6。 |
| PA5 | SCK | SPI1_SCK | 三个 TMC5160A 共用 `SCK_CFG2`。 |
| PA6 | MISO | SPI1_MISO | 三个 TMC5160A 共用 `SDO_CFG0`。 |
| PA7 | MOSI | SPI1_MOSI | 三个 TMC5160A 共用 `SDI_CFG1`。 |
| PA10 | ZLIM3 | GPIO 输入 / EXTI10 | Z 轴限位 3，U17 光耦输出。 |
| PA11 | ZLIM2 | GPIO 输入 / EXTI11 | Z 轴限位 2，U14 光耦输出。 |
| PA12 | ZLIM1 | GPIO 输入 / EXTI12 | Z 轴限位 1，U10 光耦输出。 |
| PA13 | SWDIO | 调试 | CN3。生产固件不可占用。 |
| PA14 | SWCLK | 调试 | CN3。生产固件不可占用。 |
| PB0 | YDIR | 输出 | Y 轴 U19 `REFR_DIR`。 |
| PB1 | YSTEP | 输出 | Y 轴 U19 `REFL_STEP`。 |
| PB4 | ZEN | 输出，低有效 | Z 轴 U20 `DRV_ENN`。 |
| PB5 | ZDIR | 输出 | Z 轴 U20 `REFR_DIR`。 |
| PB6 | ZSTEP | 输出 | Z 轴 U20 `REFL_STEP`。 |
| PB7 | ZCS | GPIO 输出 | Z 轴 U20 `CSN_CFG3`，低有效片选。 |
| PB8 | SCL | I2C1_SCL | MCP4728 U11。 |
| PB9 | SDA | I2C1_SDA | MCP4728 U11。 |
| PB12 | VOUT_5 | GPIO 输出 | 控制 CN1 的 +5V 高边开关；输出高电平使能。 |
| PB13 | VOUT_24 | GPIO 输出 | 控制 CN5 的 +24V Z 轴抱闸高边开关；输出高电平使能。 |
| PB14 | XLIM3 | GPIO 输入 / EXTI14 | X 轴限位 3，U15 光耦输出。 |
| PB15 | XLIM2 | GPIO 输入 / EXTI15 | X 轴限位 2，U12 光耦输出。 |
| PC2 | XCS | GPIO 输出 | X 轴 U18 `CSN_CFG3`，低有效片选。 |
| PC3 | XEN | 输出，低有效 | X 轴 U18 `DRV_ENN`。 |
| PC4 | YCS | GPIO 输出 | Y 轴 U19 `CSN_CFG3`，低有效片选。 |
| PC5 | YEN | 输出，低有效 | Y 轴 U19 `DRV_ENN`。 |
| PC6 | XLIM1 | GPIO 输入 / EXTI6 | X 轴限位 1，U8 光耦输出。 |
| PC7 | YLIM3 | GPIO 输入 / EXTI7 | Y 轴限位 3，U16 光耦输出。 |
| PC8 | YLIM2 | GPIO 输入 / EXTI8 | Y 轴限位 2，U13 光耦输出。 |
| PC9 | YLIM1 | GPIO 输入 / EXTI9 | Y 轴限位 1，U9 光耦输出。 |
| PC10 | TX1 | USART3_TX | MAX3232 U5 第一路发送输入，外部接口 CN4。 |
| PC11 | RX1 | USART3_RX | MAX3232 U5 第一路接收输出，外部接口 CN4。 |
| PC13 | LDAC | GPIO 输出 | MCP4728 `LDAC#`，低电平更新 DAC 输出。 |
| PC14 | LED | GPIO 输出 | 板载 LED；用户确认。 |
| PC15 | RDY | GPIO 输入 | MCP4728 `RDY/BSY#`，低电平表示器件忙。 |

未标入表中的 MCU 引脚在该原理图上为未连接、保留、供电/复位/晶振引脚，或未见功能网络标注；不要在未复核硬件后分配其功能。

## 三轴 TMC5160A 与 SPI

| 轴 | 驱动器 | CS | STEP | DIR | EN | 电机接口 |
| --- | --- | --- | --- | --- | --- | --- |
| X | U18 `TMC5160A-TA-T` | PC2/XCS | PA1/XSTEP | PA0/XDIR | PC3/XEN | CN17，四线。 |
| Y | U19 `TMC5160A-TA-T` | PC4/YCS | PB1/YSTEP | PB0/YDIR | PC5/YEN | CN18，四线。 |
| Z | U20 `TMC5160A-TA-T` | PB7/ZCS | PB6/ZSTEP | PB5/ZDIR | PB4/ZEN | CN19，四线。 |

### SPI 固定连接

三个驱动器共用 SPI1 总线：PA5/SCK → `SCK_CFG2`，PA7/MOSI → `SDI_CFG1`，PA6/MISO ← `SDO_CFG0`。片选必须在软件中逐轴独立拉低，且 `CSN_CFG3` 为低有效。

每个轴的 `DRV_ENN` 均为低有效：上电和故障处理时，应在正确配置驱动器前保持高电平禁用。图中 `DIAG0_SWN`、`DIAG1_SWP` 没有连接到 MCU，不能依赖 TMC5160 的 DIAG 中断；若需要堵转/故障中断，必须先修改硬件。

### 功率级与电流检测

- 每轴由 8 颗 `BSC072N08NS5` 外置 N-MOSFET 组成两相全桥，栅极串联 10 Ω。
- 每轴使用两只 50 mΩ 电阻并联作为电流采样分流电阻，等效约 **25 mΩ**。
- `VS/VSA` 使用 +24V；各 TMC5160 有独立的电荷泵、5VOUT/12VOUT、VCC 和去耦网络。
- 电机额定电流、斩波模式、细分、静止电流和速度/加速度并未由原理图确定，必须由 TMC5160 SPI 寄存器配置并结合实机测量确认。

## 限位输入与中断

所有限位接口都采用 LTV-817 光耦隔离：外侧为 +24V 供电的三针接口，内侧为 +3V3 上拉、1 kΩ 串联与 100 nF 滤波后送入 MCU。光耦导通时将 MCU 侧信号拉低，因此限位触发的电平为 **低有效**。

| 轴 | 限位 1 | 限位 2 | 限位 3 | 推荐 EXTI IRQ 分组 |
| --- | --- | --- | --- | --- |
| X | CN7 → PC6/EXTI6，负限位 | CN11 → PB15/EXTI15，零点 | CN14 → PB14/EXTI14，正限位 | EXTI9_5、EXTI15_10 |
| Y | CN8 → PC9/EXTI9，负限位 | CN12 → PC8/EXTI8，零点 | CN15 → PC7/EXTI7，正限位 | EXTI9_5 |
| Z | CN9 → PA12/EXTI12，负限位 | CN13 → PA11/EXTI11，零点 | CN16 → PA10/EXTI10，正限位 | EXTI15_10 |

注意事项：

- 九个信号占用的 EXTI 线为 6–15，线号没有重复，能够同时映射。
- 推荐配置下降沿 EXTI（触发时由高变低），但这是由电路电平推导出的软件建议，不是图纸中明确的 NVIC 配置；应在实机确认传感器常态和线缆极性后启用。
- ISR 只记录轴号/限位号和立即停止步进所需的最小动作；减速、回零状态机和通信上报应在主循环或任务上下文中执行。

## 通信、DAC 与辅助输出

| 功能 | 器件 / 接口 | MCU 连接 | 说明 |
| --- | --- | --- | --- |
| RS232-1 | U5 MAX3232，CN4 | PC10/TX1、PC11/RX1 | 3.3V MCU UART 转 RS232；保留给后续扩展。 |
| RS232-2 | U5 MAX3232，CN6 | PA2/TX2、PA3/RX2 | 3.3V MCU UART 转 RS232；当前上位机通信接口。 |
| 四路灯板控制 | U11 MCP4728、U1/U2 TLV9062、CN10 | PB8/SCL、PB9/SDA、PC13/LDAC、PC15/RDY | 四通道 DAC 经运放电压跟随缓冲并以 100 Ω 串联输出。DAC 供电为 +3V3；原图无增益网络，软件应按约 0–3.3V 输出范围设计，除非实测证实外部另有变换。 |
| 5V 继电器输出 | CN1 | PB12/VOUT_5 | P-MOS 高边开关；CN1-1 为受控 +5V，CN1-3 为 GND，CN1-2 未连接。 |
| Z 轴抱闸输出 | CN5 | PB13/VOUT_24 | P-MOS 高边开关；CN5-1 为受控 +24V，CN5-3 为 GND，CN5-2 未连接。PB13 输出高电平时输出 24V、抱闸释放；低电平时抱闸。 |

## 固件配置检查清单

1. SPI1：PA5/PA6/PA7；三个 CS（PC2、PC4、PB7）设为推挽输出，默认高电平。
2. 三路 EN（PC3、PC5、PB4）设为推挽输出，默认高电平以禁用 TMC5160。
3. STEP/DIR 共六路设为推挽输出；STEP 的脉宽、频率与方向建立时间按 TMC5160 数据手册配置。
4. 九路限位设为输入并按低有效处理；如使用 EXTI，配置 EXTI9_5 和 EXTI15_10 的共享中断服务函数。
5. I2C1 使用 PB8/PB9，采用开漏方式和外部上拉；PC13 作为 LDAC 输出，PC15 作为 RDY 输入。
6. USART3 使用 PC10/PC11，USART2 使用 PA2/PA3；波特率、帧格式和协议由上位机定义。
7. PB12、PB13 上电默认低电平，避免无条件接通继电器或抱闸电源。

## 待确认项

- 每个限位接口所连接传感器的常开/常闭逻辑；机械含义已确认：LIM1 为负限位、LIM2 为零点、LIM3 为正限位。
- 四路 DAC 输出所需的实际电压范围；原图标题为“灯板5V控制”，但电路本身未显示从 3.3V DAC 到 5V 的增益。
- CN17/CN18/CN19 上两相绕组的线序，接线前须与电机线束定义核对。
- TMC5160 的具体寄存器参数与故障策略；原图未将 DIAG 信号引到 MCU。

## 已确认的电机与抱闸信息

| 轴 | 电机型号 | 额定相电流 | 抱闸 |
| --- | --- | --- | --- |
| X | PKP268D42A2 | 4.2 A/相 | 无。 |
| Y | PKP268D42A2 | 4.2 A/相 | 无。 |
| Z | PKP268D28M2 | 2.8 A/相 | CN5 提供 24V 时释放，断电时抱闸。 |
