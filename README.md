# MUEYE MotorContrl

STM32F103RC 三轴电机控制固件，X/Y/Z 分别使用独立的 TMC5160、DIR、STEP 和限位输入，采用
Keil µVision + Arm Compiler 6 构建。开发过程遵循“先验证通信，再验证寄存器、
使能、单步和短时运动，最后再接入控制协议”的最小验证路线。

## 目录

- `MotorContrl/Src`：系统入口、时钟及保留的中断代码。
- `MotorContrl/Inc`：公共接口、参数类型与配置宏。
- `MotorContrl/APPs`：固定轮询、串口、运动、辅助输出、限位、LED 与灯板控制。
- `MotorContrl/Drivers/Board`：板级外设驱动；其余 `Drivers` 子目录为未修改的 HAL/CMSIS。
- `MotorContrl/MDK-ARM`：Keil 工程文件。
- `doc/schematic_info.md`：硬件引脚和连接关系的唯一参考。
- `doc/risk_and_development_plan.md`：风险、开发路线和实机验证记录。
- [重构架构与保留边界](doc/refactor_architecture.md)、[逐项测试方案](doc/refactor_test_plan.md)。
- `tests/check_refactor.py`：以重构前工作区为基线的结构检查和主机 C 差分测试。
- `firmware_releases`：按标签归档的 HEX 文件；文件名含 `unverified` 的快照尚未完成实机验收。

## 版本路线

当前工作区已将 X 轴运动状态机通过轴配置结构体扩展到 Y/Z，灯板串口控制和 MCP4728 易失寄存器写入也已实现。三轴代码已通过主机模拟与 Keil 构建，但 Y/Z 运动尚未完成实机验收；本轮改动归档为 `dev-20260903-three-axis-protocol-unverified` 开发快照，不作为实机通过版本。验证结果和上板清单见 [重构测试方案](doc/refactor_test_plan.md)。

下表中的“本版功能”是该标签对应代码的实际范围；每个标签都保留了完整源代码，
可以用 Git 切换回当时的状态。

| 标签 | 相对前一归档版本的主要改动 | 本版功能与验证状态 |
|---|---|---|
| `v0.3-gstat-clear-pass` | 建立 TMC5160 启动后的状态读取和 GSTAT 清除步骤。 | 读取 SPI 身份，读取并清除 GSTAT 锁存位；等待后确认 UV_CP 未再次出现。 |
| `v0.4-chopconf-read-pass` | 在 GSTAT 验证后增加 CHOPCONF 静态读回。 | 读取并记录复位值 `0x10410150`，不使能驱动、不发 STEP。 |
| `v0.5-low-current-config-pass` | 增加最小电流和保守斩波配置写入。 | `IHOLD_IRUN=0`，保留 CHOPCONF 其他位并将 TOFF 设为 3；读回 `0x10410153`。 |
| `v0.6-enable-no-step-pass` | 在配置通过后增加驱动使能和状态检查。 | 低电流使能 X 轴，读取 GSTAT/DRV_STATUS；无故障保持约 5 秒后自动禁用，不发 STEP。 |
| `v0.7-single-step-pass` | 增加一次受控 STEP，并临时将 PA1 从 TIM2 复用输出切换为 GPIO。 | 发送 1 个 1 ms 高、1 ms 低脉冲；MSCNT 变化 `+1`，随后恢复 PA1 复用配置。 |
| `v0.8-sixteen-step-pass` | 在单步通过后增加有限脉冲批次。 | 固定方向，发送 16 个约 20 Hz 脉冲；MSCNT 变化 `+16`，确认输入通路连续可靠。 |
| `v0.9-low-current-motion-pass` | 将计数测试扩展为定时短时运动。 | 最小电流下约 500 Hz 运行 4 秒，理论约 14°；手感确认电机连续运动，结束后自动禁用。 |
| `v0.10-limit-static-input-deferred` | 暂停电机运动验证，加入限位输入静态读取框架。 | 配置 9 路限位输入，暂不启用 EXTI；因现有限位光电开关损坏，静态输入尚未宣称实机通过，因此没有对应 HEX。 |
| `v0.11-serial-link-test-pass` | 增加首条串口链路测试命令 `0x0100`（线上字节 `01 00`）及固定应答。 | CN4/USART3 收到完整 14 字节测试帧，回传帧正确；仅验证串口链路，不驱动电机。 |
| `v0.12-cn4-primary-interface` | 根据实机接线结果冻结 CN4 为主机串口。 | 使用 USART3（PC10/PC11，115200 8N1）；CN6/USART2 保留作后续扩展。 |
| `v0.13-cn4-serial-link-pass` | 将已通过验证的 CN4 串口版本正式打包归档，并补充匹配 HEX。 | 实机验证：收到 14 字节，最后字节 `0xAA`，UART 错误计数为 0，回传正确；限位硬件仍暂缓，未进行电机运动。 |
| `v0.14-10khz-1a-motion-pass` | 在串口链路版本上恢复 X 轴有限运动测试，将 STEP 提升到 10 kHz，并将运行电流设为约 1 A。 | `IRUN=3`、`IHOLD=0`；10 kHz 运行 1.2 秒、约 12000 脉冲，理论约 84.4°；用户实机确认电机运动正常。限位中断和串口运动命令仍未启用。 |
| `v0.15-serial-x-motion-pass` | 增加 CN4/USART3 的 `0x0200` X 轴运动命令、TIM2 可变 STEP 频率、精确脉冲计数和 `0x0201` 完成回传；修复重复命令时 PA1 未恢复复用输出的问题。 | 仅支持 X 轴、1000–10000 Hz、1–12000 STEP、单次不超过 5 秒；用户实机验证 1000 Hz/1000 STEP 与 10000 Hz/10000 STEP，并确认同一命令可重复运动。Keil 构建 0 错误、0 警告。 |
| `dev-20260827-limit-protection-unverified` | 增加 `0x0202` 主机停止、`0x0203` 状态查询、验证用 `0x02F0` 持续运行，以及 X 轴三路限位轮询停机。 | 代码与 Keil 构建已验证；限位开关未到货，PC6/PB15/PB14 限位触发尚未实机验证。本快照不作为正式发布版本，不提供已验证 HEX。 |
| `dev-20260827-serial-aux-output-unverified` | 增加 `0x0300` 5 V 继电器和 `0x0400` 抱闸输出的串口开关控制。 | 用户确认串口控制功能和回传正常；抱闸器件仍有硬件问题，限位开关未到货，因此不作为正式发布版本，不提供已验证 HEX。 |
| `v0.16-layered-refactor-aux-output-pass` | 按原功能完成 `Inc/Src/Drivers/APPs` 分层，保留 CN4/USART3 主通信、CN6/USART2 扩展、运动、限位轮询和辅助输出；增加灯板/MCP4728 空接口。 | Keil 构建 0 错误、0 警告；重构主机差分测试默认配置 49 组、自检配置 2 组通过；用户报告现有功能实机测试暂时无误，并确认更换器件后 PB13/CN5 24 V 抱闸输出正常。光电开关将在下一聊天中测试，MCP4728/I2C 尚未实现或验证。 |
| `dev-20260901-mcp4728-serial-unverified` | 增加 `0x0500` 灯板串口命令，按协议控制 MCP4728 A/B/C/D 四路易失 DAC 输出开启或关闭。 | Keil 构建 0 错误、0 警告；MCP4728 ACK、输出电压、四路映射及 SCL/SDA 电平尚未实机测试，待改板将 I2C 上拉改为 5 V 后验证。本快照不作为正式发布版本。 |
| `dev-20260831-limit-pnp-active-high-unverified` | 根据 EE-SX951P-R PNP OUT1 接线，将限位采样统一改为 MCU 高电平有效；保留 10 ms 轮询，暂不启用 EXTI 和新的限位停机测试。 | 代码和接线记录已更新；等待用户完成正常、遮光、断线、掉电及电阻温升实测；未形成已验证 HEX。 |
| `dev-20260901-limit-exti-motion-stop-unverified` | 将九路限位改为双边沿 EXTI；X 轴三路遮光上升沿在 ISR 中立即停止 X 轴 STEP，但保持 XEN 使能，Y/Z 限位只记录各自状态，并由主循环完成限位停止状态和串口错误应答。 | PC6 输入极性已完成实测；EXTI、X 轴限位停机及其余八路仍待上板验证。本快照不作为正式发布版本。 |
| `dev-20260901-limit-exti-motion-stop-pass` | 在上述限位保护基础上补全当前通信协议，并记录 X 轴连续运动中限位中止实测结果。 | 用户实测连续运行正常；遮挡 X 轴限位可中止运动并回传 `0x02FF/0x0B`。普通停止保持使能和 MCP4728 尚未分别完成实机验收；仍为开发快照。 |
| `dev-20260903-three-axis-protocol-unverified` | 将运动、停止、状态查询和限位处理按轴配置结构体复用到 X/Y/Z；删除旧 X 轴自动自检及专用包装；补全三轴通信协议文档。 | Keil 构建 0 错误、0 警告；三轴主机模拟通过；Y/Z 电机与限位、Z 轴 `IHOLD=0`/`IRUN=3`、MCP4728 I2C 尚未完成完整实机验收。本快照标记为未测试开发归档。 |

## 当前硬件与安全状态

- X 轴 TMC5160：SPI1，CS 为 PC2，ENN 为 PC3，DIR 为 PA0，STEP 为 PA1。
- Y 轴 TMC5160：SPI1，CS 为 PC4，ENN 为 PC5，DIR 为 PB0，STEP 为 PB1（TIM3_CH4）。
- Z 轴 TMC5160：SPI1，CS 为 PB7，ENN 为 PB4，DIR 为 PB5，STEP 为 PB6（TIM4_CH1）；运动前需先释放 PB13/CN5 抱闸。
- X 电机接在 CN17；同一绕组接在同一相输入，绕组极性可通过 DIR/后续逻辑调整。
- 当前 v0.15 X 轴串口运动使用约 1.15 A RMS/相运行电流、约 0.29 A RMS/相保持电流，仅适合空载、短时测试；不要直接用于长期连续运行或大负载。
- PB12/PB13 的串口开关控制已归档；用户确认串口控制和回传正常，并确认更换器件后 PB13/CN5 的 24 V 抱闸输出正常。
- 限位开关已到货；PC6 接线、电气电平和高有效极性已确认，EXTI 与限位停机仍只允许在脱开机构或断开电机功率的条件下验证。
- 当前限位保护已完成 X 轴连续运动中限位中止的实机验证；其余通道、完整停机时序和掉电/断线测试仍需继续验收。

## 查看和构建

切换到某个历史版本，例如：

```text
git switch --detach v0.8-sixteen-step-pass
```

在 Keil 中打开 `MotorContrl/MDK-ARM/MotorContrl.uvprojx`，执行 **Build Target**。
对应的已验证 HEX 位于 `firmware_releases`。返回最新开发分支：

```text
git switch master
```

当前主机通信使用 CN4/USART3；CN6/USART2 保留作后续扩展。运动帧轴号 `01/02/03` 分别选择 X/Y/Z；每条命令只运行一轴，不做多轴同步插补，也不允许另一轴在当前运动完成前启动。`0x0200` 请求帧的速度单位为 STEP/s、距离单位为 STEP 脉冲数，正常完成后回传 `0x0201` 并保持该轴驱动使能；`0x0202` 只停止帧中指定的当前活动轴，`0x0203` 查询当前状态。验证用 `0x02F0` 为单轴持续运行命令。

每轴只响应自己的三路限位：X 为 PC6/PB15/PB14，Y 为 PC9/PC8/PC7，Z 为 PA12/PA11/PA10。当前三轴限位保护均接入公共启动拦截和运动中停机链路，Z 轴通过 `Z_LIMIT_PROTECTION_ENABLED=1` 开启；Z 轴限位和降低电流参数仍待上板验证。Z 轴当前使用 `IHOLD=0`、`IRUN=3` 测试参数，尚未完成硬件验收。

开发快照 `dev-20260827-serial-aux-output-unverified` 还支持 `0x0300`（PB12/CN1 5 V）和 `0x0400`（PB13/CN5 24 V 抱闸）控制：
数据前两字节为 `01 00` 开启或 `02 00` 关闭，其余 6 字节为 0；合法帧回传 `AA AA` 帧头、
原命令与数据、`55 55` 帧尾。PB13 输出高电平会释放抱闸；串口控制、回传以及更换器件后的 24 V 抱闸输出已由用户确认。

下一项工作是按 Y、Z 顺序进行低速短行程空载测试，并逐路验证对应限位；Z 轴每次测试前先释放抱闸，结束后再抱闸。当前 `0x0500` 已支持按协议控制 CN10 四路输出，但 MCP4728 ACK、电压和 SCL/SDA 电平仍需等 I2C 上拉改为 5 V 后验证。
