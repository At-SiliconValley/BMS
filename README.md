# BMS 电池管理系统 — 项目 Wiki

> **项目地址**: https://github.com/At-SiliconValley/BMS  
> **项目名称**: 尚硅谷电池管理项目 (BMS)  
> **主控芯片**: STM32F103C8T6 (Cortex-M3)  
> **开发环境**: Keil MDK-ARM + STM32CubeMX  
> **实时系统**: FreeRTOS  

---

## 📑 目录

1. [项目概述](#1-项目概述)
2. [系统架构](#2-系统架构)
3. [01_BMS_PRO — BMS主控端](#3-01_bms_pro--bms主控端)
4. [02_gateway — 通信网关端](#4-02_gateway--通信网关端)
5. [硬件依赖](#5-硬件依赖)
6. [软件依赖](#6-软件依赖)
7. [目录结构详解](#7-目录结构详解)
8. [通信协议](#8-通信协议)
9. [快速开始](#9-快速开始)
10. [API参考](#10-api参考)
11. [开发计划](#11-开发计划)
12. [常见问题](#12-常见问题)

---

## 1. 项目概述

本项目是**尚硅谷**推出的嵌入式实战教学项目，实现了一套完整的**电池管理系统 (Battery Management System, BMS)**。系统采用**主从架构**：

- **BMS主控端** (`01_BMS_PRO`)：负责电池组的实时监测、保护控制与本地显示
- **通信网关端** (`02_gateway`)：负责数据汇聚、协议转换与云端通信

### 核心功能

| 功能模块     | 说明                                                  |
| ------------ | ----------------------------------------------------- |
| 🔋 电池监测   | 通过TI BQ769系列AFE芯片采集单体电压、总压、电流、温度 |
| ⚡ 充放电保护 | 过压、欠压、过流、过温、短路保护                      |
| 📊 SOC估算    | 基于安时积分法的电量状态估算                          |
| 📺 本地显示   | 0.96寸 OLED (SSD1315) 实时显示电池参数                |
| 🔌 网关通信   | 支持CAN总线、以太网(W5500)、MQTT、Modbus RTU          |
| ☁️ 云端上传   | 通过MQTT将电池数据上传至物联网平台                    |

---

## 2. 系统架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        系统架构图                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   ┌──────────────┐         CAN总线         ┌──────────────┐    │
│   │  01_BMS_PRO  │◄───────────────────────►│ 02_gateway   │    │
│   │   BMS主控端   │                         │   网关端      │    │
│   └──────┬───────┘                         └──────┬───────┘    │
│          │                                        │             │
│          ▼                                        ▼             │
│   ┌──────────────┐                         ┌──────────────┐    │
│   │ BQ769 AFE芯片 │                         │  W5500以太网  │    │
│   │ (电压/电流/   │                         │   模块       │    │
│   │  温度采集)    │                         └──────┬───────┘    │
│   └──────┬───────┘                                │             │
│          │                                   ┌────┴────┐       │
│          ▼                                   ▼         ▼       │
│   ┌──────────────┐                     ┌────────┐ ┌────────┐  │
│   │  SSD1315 OLED │                     │  MQTT  │ │ Modbus │  │
│   │   显示屏      │                     │  Broker│ │  主站  │  │
│   └──────────────┘                     └────────┘ └────────┘  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 数据流

```
电池组 → BQ769 AFE → STM32F103 (BMS主控) → CAN总线 
                                                  ↓
云端平台 ← MQTT Broker ← W5500以太网 ← STM32F103 (网关)
                                                  ↓
                                            Modbus RTU (可选)
```

---

## 3. 01_BMS_PRO — BMS主控端

### 3.1 功能描述

BMS主控端是系统的核心采集与控制单元，主要承担以下职责：

- **电池参数采集**：通过I2C总线与TI BQ769系列AFE通信，获取：
  - 各串联电芯电压 (Cell Voltage)
  - 电池组总电压 (Pack Voltage)
  - 充放电电流 (Current)
  - 电芯温度 (Temperature)
- **电池状态估算**：实现SOC (State of Charge) 估算
- **安全保护**：过压保护(OVP)、欠压保护(UVP)、过流保护(OCP)、过温保护(OTP)
- **本地显示**：通过I2C驱动SSD1315 OLED显示屏，实时展示电池状态
- **调试输出**：通过USART输出调试信息

### 3.2 软件层次

```
┌─────────────────────────────────────────┐
│              App 应用层                  │
│  ┌──────────┐ ┌──────────┐ ┌─────────┐ │
│  │App_Battery│ │App_Display│ │App_Main │ │
│  │ 电池管理  │ │ 显示驱动  │ │ 主循环  │ │
│  └────┬─────┘ └────┬─────┘ └────┬────┘ │
├───────┼────────────┼────────────┼──────┤
│       ▼            ▼            ▼      │
│              Com 通用层                 │
│  ┌──────────┐ ┌──────────┐ ┌─────────┐ │
│  │Com_BQ769 │ │Com_Debug │ │Com_Delay│ │
│  │ AFE通信  │ │ 调试输出  │ │ 延时函数 │ │
│  └────┬─────┘ └────┬─────┘ └────┬────┘ │
├───────┼────────────┼────────────┼──────┤
│       ▼            ▼            ▼      │
│              Int 驱动层                 │
│  ┌──────────┐ ┌─────────────────────┐  │
│  │ Int_BQ769│ │     Int_OLED        │  │
│  │AFE底层驱动│ │  SSD1315显示驱动     │  │
│  └──────────┘ └─────────────────────┘  │
├─────────────────────────────────────────┤
│              Core HAL层                 │
│         (STM32CubeMX生成)               │
│     I2C / USART / GPIO / TIM           │
├─────────────────────────────────────────┤
│              MId 中间件层               │
│            FreeRTOS V10.x              │
│     任务调度 / 队列 / 信号量 / 事件组   │
├─────────────────────────────────────────┤
│           Drivers 芯片驱动层            │
│      STM32F1xx_HAL_Driver + CMSIS     │
└─────────────────────────────────────────┘
```

### 3.3 关键模块说明

#### App_Battery
- **职责**: 电池核心管理逻辑
- **功能**: 
  - 周期性读取BQ769数据
  - SOC估算 (安时积分法)
  - 保护阈值判断与MOSFET控制
  - 数据打包发送至显示模块

#### App_Display
- **职责**: OLED显示界面管理
- **功能**:
  - 电压/电流/温度/SOC数值显示
  - 电池状态图标绘制
  - 报警信息弹窗显示

#### Com_BQ769 / Int_BQ769
- **职责**: BQ769 AFE芯片通信驱动
- **接口**: I2C (标准模式 100kHz / 快速模式 400kHz)
- **关键寄存器操作**:
  - 电压寄存器 (VC1~VC16)
  - 电流寄存器 (CC)
  - 温度寄存器 (TS1/TS2/TS3)
  - 状态/保护寄存器 (SYS_STAT / PROTECT)

---

## 4. 02_gateway — 通信网关端

### 4.1 功能描述

网关端是系统的数据汇聚与转发枢纽，承担以下职责：

- **CAN总线通信**：接收BMS主控端发送的电池数据帧
- **以太网通信**：通过W5500芯片实现TCP/IP网络接入
- **MQTT客户端**：将电池数据以JSON格式发布到MQTT Broker
- **Modbus RTU**：支持Modbus协议，可被上位机/PLC读取
- **协议转换**：CAN帧 ↔ JSON ↔ Modbus寄存器映射

### 4.2 软件层次

```
┌─────────────────────────────────────────┐
│              App 应用层                  │
│  ┌──────────┐ ┌──────────────────────┐  │
│  │ App_Main │ │     App_Task1        │  │
│  │ 主任务   │ │  数据采集/协议转换    │  │
│  └────┬─────┘ └──────────┬───────────┘  │
├───────┼──────────────────┼──────────────┤
│       ▼                  ▼              │
│              Com 通用层                 │
│  ┌──────────┐ ┌──────────┐ ┌─────────┐ │
│  │Com_Debug │ │Com_Delay │ │  cJSON  │ │
│  │ 调试输出  │ │ 延时函数  │ │JSON解析  │ │
│  └──────────┘ └──────────┘ └─────────┘ │
├─────────────────────────────────────────┤
│              Int 驱动层                 │
│  ┌────────┐ ┌────────┐ ┌────────┐     │
│  │Int_CAN │ │Int_W5500│ │Int_Modbus│   │
│  │CAN驱动 │ │以太网驱动│ │Modbus协议│   │
│  └────────┘ └────────┘ └────────┘     │
│  ┌─────────────────────────────────┐    │
│  │        Int_MQTT_Client          │    │
│  │    MQTT客户端 (基于MQTTPacket)  │    │
│  └─────────────────────────────────┘    │
├─────────────────────────────────────────┤
│              Core HAL层                 │
│         (STM32CubeMX生成)               │
│     CAN / SPI / USART / GPIO / TIM     │
├─────────────────────────────────────────┤
│              Mid 中间件层               │
│            FreeRTOS V10.x              │
├─────────────────────────────────────────┤
│           Drivers 芯片驱动层            │
│      STM32F1xx_HAL_Driver + CMSIS     │
└─────────────────────────────────────────┘
```

### 4.3 关键模块说明

#### Int_CAN
- **职责**: CAN总线数据接收
- **配置**: 
  - 波特率: 500 kbps (可配置)
  - 接收过滤器: 仅接收BMS主控端数据帧
- **数据帧格式**:
  ```
  CAN ID: 0x180 (BMS → Gateway)
  DLC: 8 bytes
  Data[0-1]: 总电压 (mV, 16bit)
  Data[2-3]: 电流 (mA, 16bit, 有符号)
  Data[4]:   最高单体电压 (10mV/位)
  Data[5]:   最低单体电压 (10mV/位)
  Data[6]:   温度1 (℃ + 40偏移)
  Data[7]:   SOC (0-100%)
  ```

#### Int_W5500 / W5500驱动
- **职责**: W5500全硬件TCP/IP以太网控制器驱动
- **接口**: SPI (最高80MHz)
- **支持协议**: TCP, UDP, IP, ICMP, ARP, PPPoE
- **Socket配置**: 
  - Socket 0: TCP客户端 (MQTT连接)
  - Socket 1: TCP服务器 (可选调试接口)

#### Int_MQTT_Client / MQTTPacket
- **职责**: MQTT协议实现
- **版本**: MQTT v3.1.1
- **功能**:
  - CONNECT / PUBLISH / SUBSCRIBE / PINGREQ
  - QoS 0/1 支持
  - 遗嘱消息 (Will Message)
- **JSON数据格式示例**:
  ```json
  {
    "device_id": "BMS_001",
    "timestamp": 1690123456,
    "pack_voltage": 48720,
    "current": -1250,
    "soc": 85,
    "max_cell_v": 4050,
    "min_cell_v": 4020,
    "temperature": 32,
    "status": "normal"
  }
  ```

#### Int_Modbus
- **职责**: Modbus RTU从站实现
- **接口**: USART (默认9600/8/N/1)
- **功能码支持**:
  - 0x03: 读取保持寄存器 (Read Holding Registers)
  - 0x06: 写单个寄存器 (Write Single Register)
- **寄存器映射**:
  | 地址  | 内容         | 单位          |
  | ----- | ------------ | ------------- |
  | 40001 | 总电压       | 0.1V          |
  | 40002 | 电流         | 0.1A (有符号) |
  | 40003 | SOC          | 1%            |
  | 40004 | 温度         | 1℃            |
  | 40005 | 最高单体电压 | mV            |
  | 40006 | 最低单体电压 | mV            |

---

## 5. 硬件依赖

### 5.1 BMS主控端 (01_BMS_PRO)

| 器件     | 型号             | 功能               | 接口   |
| -------- | ---------------- | ------------------ | ------ |
| 主控MCU  | STM32F103C8T6    | 核心处理器         | -      |
| 电池AFE  | TI BQ76920/30/40 | 电压/电流/温度采集 | I2C    |
| 显示屏   | SSD1315 (128x64) | 本地参数显示       | I2C    |
| 调试串口 | USB转TTL模块     | 日志输出           | USART1 |

### 5.2 网关端 (02_gateway)

| 器件       | 型号          | 功能           | 接口   |
| ---------- | ------------- | -------------- | ------ |
| 主控MCU    | STM32F103C8T6 | 核心处理器     | -      |
| 以太网模块 | W5500         | TCP/IP网络接入 | SPI    |
| CAN收发器  | TJA1050       | CAN总线物理层  | CAN    |
| 调试串口   | USB转TTL模块  | 日志输出       | USART1 |

### 5.3 系统连接图

```
                    BMS主控端 (01_BMS_PRO)
                    ┌─────────────────┐
    电池组 ────────►│  BQ769 AFE芯片  │
    (3~16串)       │  (电压/电流/温度) │
                    └────────┬────────┘
                             │ I2C
                    ┌────────▼────────┐
                    │  STM32F103C8    │
                    │   + FreeRTOS    │
                    └────────┬────────┘
                             │ USART (Debug)
                             │ I2C (OLED)
                             │ CAN
                             ▼
                    ┌─────────────────┐
                    │   TJA1050       │
                    │  CAN收发器      │
                    └────────┬────────┘
                             │ CAN_H / CAN_L
                             ▼
                    ┌─────────────────┐
                    │   TJA1050       │
                    │  CAN收发器      │
                    └────────┬────────┘
                             │ CAN
                    ┌────────▼────────┐
                    │  STM32F103C8    │
                    │   + FreeRTOS    │
                    └────────┬────────┘
                             │ SPI
                    ┌────────▼────────┐
                    │     W5500       │
                    │  以太网控制器    │
                    └────────┬────────┘
                             │ RJ45
                             ▼
                         路由器/交换机
                             │
                             ▼
                       MQTT Broker
                       (EMQX / Mosquitto)
```

---

## 6. 软件依赖

### 6.1 开发工具链

| 工具            | 版本          | 用途               |
| --------------- | ------------- | ------------------ |
| Keil MDK-ARM    | μVision 5.38+ | 编译/调试/烧录     |
| STM32CubeMX     | 6.9.0+        | 芯片初始化代码生成 |
| ST-Link Utility | 4.6+          | 固件烧录           |
| 串口调试助手    | 任意          | 日志查看           |

### 6.2 软件库

| 库                   | 版本   | 来源         | 用途           |
| -------------------- | ------ | ------------ | -------------- |
| STM32F1xx_HAL_Driver | 1.1.9  | ST官方       | HAL底层驱动    |
| CMSIS                | 5.4.0  | ARM          | 内核接口       |
| FreeRTOS             | 10.4.6 | Amazon       | 实时操作系统   |
| cJSON                | 1.7.15 | Dave Gamble  | JSON解析/生成  |
| MQTTPacket           | 1.0.0  | Eclipse Paho | MQTT协议栈     |
| W5500驱动            | 1.0.0  | WizNet       | 以太网芯片驱动 |

### 6.3 FreeRTOS配置

```c
// FreeRTOSConfig.h 关键配置
#define configUSE_PREEMPTION            1
#define configUSE_IDLE_HOOK             0
#define configUSE_TICK_HOOK             0
#define configCPU_CLOCK_HZ              72000000    // 72MHz
#define configTICK_RATE_HZ              1000        // 1ms tick
#define configMAX_PRIORITIES            5
#define configMINIMAL_STACK_SIZE        128
#define configTOTAL_HEAP_SIZE           8192        // 8KB 堆内存
#define configMAX_TASK_NAME_LEN         16
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         1
#define configUSE_MUTEXES               1
#define configQUEUE_REGISTRY_SIZE       8
#define configCHECK_FOR_STACK_OVERFLOW  2
#define configUSE_MALLOC_FAILED_HOOK    1
```

---

## 7. 目录结构详解

```
BMS/
├── .gitattributes
├── README.md
│
├── 01_BMS_PRO/                    # BMS主控端工程
│   ├── 01_BMS_PRO.ioc             # STM32CubeMX配置文件
│   ├── .mxproject                 # CubeMX工程元数据
│   │
│   ├── App/                       # 应用层代码
│   │   ├── App_Battery.c/h      # 电池管理核心逻辑
│   │   ├── App_Display.c/h      # OLED显示界面
│   │   └── App_Main.c/h         # 主任务入口
│   │
│   ├── Com/                       # 通用/通信层
│   │   ├── Com_BQ769.c/h        # BQ769 AFE通信接口
│   │   ├── Com_Debug.c/h        # 调试日志输出
│   │   └── Com_Delay.c/h        # 延时函数封装
│   │
│   ├── Core/                      # HAL核心层 (CubeMX生成)
│   │   ├── Inc/                 # 头文件
│   │   │   ├── main.h
│   │   │   ├── gpio.h
│   │   │   ├── i2c.h
│   │   │   ├── usart.h
│   │   │   ├── stm32f1xx_it.h
│   │   │   └── stm32f1xx_hal_conf.h
│   │   └── Src/                 # 源文件
│   │       ├── main.c           # 主函数
│   │       ├── gpio.c           # GPIO初始化
│   │       ├── i2c.c            # I2C初始化
│   │       ├── usart.c          # USART初始化
│   │       ├── stm32f1xx_it.c   # 中断服务函数
│   │       ├── stm32f1xx_hal_msp.c
│   │       └── system_stm32f1xx.c
│   │
│   ├── Int/                       # 外设驱动层
│   │   ├── BQ769/               # BQ769 AFE驱动
│   │   │   ├── Int_BQ769.c/h
│   │   │   └── Int_BQ769_BSP.h  # 板级支持包
│   │   └── SSD1315/             # OLED显示屏驱动
│   │       ├── Int_OLED.c/h
│   │       └── Int_OLED_FONT.h  # 字库数据
│   │
│   ├── MId/                       # 中间件 (FreeRTOS)
│   │   └── FreeRTOS/
│   │       ├── include/         # 头文件
│   │       ├── portable/        # 移植层 (RVDS/ARMCC)
│   │       └── source/          # 内核源码
│   │
│   ├── Drivers/                   # 芯片官方驱动
│   │   ├── CMSIS/               # ARM内核接口
│   │   └── STM32F1xx_HAL_Driver/ # HAL库
│   │
│   └── MDK-ARM/                   # Keil工程文件
│       ├── 01_BMS_PRO.uvprojx   # 工程主文件
│       ├── 01_BMS_PRO.uvoptx    # 工程选项
│       ├── startup_stm32f103xb.s # 启动汇编
│       └── 01_BMS_PRO/          # 编译输出目录
│           ├── 01_BMS_PRO.hex   # 烧录文件
│           └── 01_BMS_PRO.axf   # 调试文件
│
└── 02_gateway/                    # 网关端工程
    ├── gateway.ioc               # STM32CubeMX配置文件
    ├── .mxproject
    │
    ├── App/                      # 应用层
    │   ├── App_Main.c/h         # 主任务
    │   └── App_Task1.c/h        # 数据采集/协议转换任务
    │
    ├── Com/                      # 通用层
    │   ├── Com_Debug.c/h
    │   ├── Com_Delay.c/h
    │   ├── cJSON.c/h            # JSON库
    │
    ├── Core/                     # HAL核心层
    │   ├── Inc/
    │   │   ├── main.h
    │   │   ├── gpio.h
    │   │   ├── can.h            # CAN接口
    │   │   ├── spi.h            # SPI接口 (W5500)
    │   │   ├── usart.h
    │   │   ├── stm32f1xx_it.h
    │   │   └── stm32f1xx_hal_conf.h
    │   └── Src/
    │       ├── main.c
    │       ├── gpio.c
    │       ├── can.c             # CAN初始化
    │       ├── spi.c             # SPI初始化
    │       ├── usart.c
    │       ├── stm32f1xx_it.c
    │       ├── stm32f1xx_hal_msp.c
    │       └── system_stm32f1xx.c
    │
    ├── Int/                      # 外设驱动层
    │   ├── CAN/                  # CAN总线驱动
    │   │   ├── Int_CAN.c/h
    │   ├── ETHERNET/             # 以太网驱动
    │   │   ├── Int_W5500.c/h
    │   │   ├── W5500/           # WizNet官方驱动
    │   │   │   ├── w5500.c/h
    │   │   ├── socket.c/h
    │   │   └── wizchip_conf.c/h
    │   ├── MODBUS/               # Modbus RTU协议
    │   │   ├── Int_Modbus.c/h
    │   │   ├── mbcrc.c/h        # CRC校验
    │   └── MQTT/                 # MQTT协议
    │       ├── Int_MQTT_Client.c/h
    │       ├── MQTTClient.c/h
    │       ├── mqtt_interface.c/h
    │       └── MQTTPacket/src/  # MQTT包解析
    │           ├── MQTTConnect.h/c
    │           ├── MQTTPublish.h/c
    │           ├── MQTTSubscribe.h/c
    │           └── ...
    │
    ├── Mid/                      # 中间件 (FreeRTOS)
    │   └── FreeRTOS/
    │       ├── include/
    │       ├── portable/
    │       └── source/
    │
    ├── Drivers/                  # 芯片官方驱动
    │   ├── CMSIS/
    │   └── STM32F1xx_HAL_Driver/
    │
    └── MDK-ARM/                  # Keil工程文件
        ├── gateway.uvprojx
        ├── gateway.uvoptx
        ├── startup_stm32f103xb.s
        └── gateway/             # 编译输出
            ├── gateway.hex
            └── gateway.axf
```

---

## 8. 通信协议

### 8.1 BMS主控 ↔ BQ769 (I2C)

| 参数      | 值                                |
| --------- | --------------------------------- |
| 接口      | I2C1 (PB6=SCL, PB7=SDA)           |
| 速率      | 100 kHz (标准模式)                |
| BQ769地址 | 0x08 (7-bit) / 0x10 (8-bit write) |
| 数据格式  | 寄存器地址 + 数据字节             |

**关键寄存器映射**:

| 寄存器地址 | 名称              | 说明              |
| ---------- | ----------------- | ----------------- |
| 0x00       | SYS_STAT          | 系统状态          |
| 0x01       | CELLBAL1          | 电芯平衡控制1     |
| 0x02       | CELLBAL2          | 电芯平衡控制2     |
| 0x03       | CELLBAL3          | 电芯平衡控制3     |
| 0x04       | SYS_CTRL1         | 系统控制1         |
| 0x05       | SYS_CTRL2         | 系统控制2         |
| 0x06       | PROTECT1          | 保护设置1         |
| 0x07       | PROTECT2          | 保护设置2         |
| 0x08       | PROTECT3          | 保护设置3         |
| 0x09       | OV_TRIP           | 过压阈值          |
| 0x0A       | UV_TRIP           | 欠压阈值          |
| 0x10~0x1D  | VC1~VC8           | 电芯电压1~8       |
| 0x2A~0x2B  | CC_HI/LO          | 库仑计数高/低字节 |
| 0x30~0x35  | ADCGAIN/ADCoffset | ADC校准           |

### 8.2 BMS主控 ↔ 网关 (CAN)

| 参数   | 值                      |
| ------ | ----------------------- |
| 接口   | CAN1 (PA11=RX, PA12=TX) |
| 波特率 | 500 kbps                |
| 模式   | 标准帧 (11-bit ID)      |

**CAN ID分配**:

| ID    | 方向          | 内容          | 周期     |
| ----- | ------------- | ------------- | -------- |
| 0x180 | BMS → Gateway | 电池基本数据  | 100ms    |
| 0x181 | BMS → Gateway | 单体电压1~4   | 200ms    |
| 0x182 | BMS → Gateway | 单体电压5~8   | 200ms    |
| 0x183 | BMS → Gateway | 温度/报警状态 | 500ms    |
| 0x280 | Gateway → BMS | 控制命令      | 事件触发 |

### 8.3 网关 ↔ 云端 (MQTT)

| 参数     | 值             |
| -------- | -------------- |
| 传输层   | TCP over W5500 |
| 协议     | MQTT v3.1.1    |
| 端口     | 1883 (默认)    |
| 客户端ID | BMS_GW_001     |
| 心跳间隔 | 60s            |

**Topic设计**:

| Topic            | 方向             | 说明                |
| ---------------- | ---------------- | ------------------- |
| `bms/001/data`   | Gateway → Broker | 电池实时数据 (JSON) |
| `bms/001/status` | Gateway → Broker | 在线状态 (retained) |
| `bms/001/cmd`    | Broker → Gateway | 远程控制命令        |
| `bms/001/ack`    | Gateway → Broker | 命令响应            |

### 8.4 网关 ↔ 上位机 (Modbus RTU)

| 参数     | 值                             |
| -------- | ------------------------------ |
| 接口     | USART2 (PA2=TX, PA3=RX)        |
| 波特率   | 9600                           |
| 格式     | 8N1 (8数据位, 无校验, 1停止位) |
| 从机地址 | 0x01                           |

---

## 9. 快速开始

### 9.1 环境准备

1. **安装 Keil MDK-ARM** (推荐 5.38 及以上版本)
2. **安装 STM32CubeMX** (用于查看/修改引脚配置)
3. **准备硬件**:
   - STM32F103C8T6 最小系统板 × 2
   - BQ76920/30/40 评估板 × 1
   - SSD1315 OLED 模块 × 1
   - W5500 以太网模块 × 1
   - TJA1050 CAN收发器模块 × 2
   - ST-Link V2 下载器 × 1
   - 3~16串锂电池组 × 1

### 9.2 编译与烧录

#### BMS主控端

```bash
# 1. 打开工程
# 使用 Keil μVision 打开 01_BMS_PRO/MDK-ARM/01_BMS_PRO.uvprojx

# 2. 编译工程
# 点击 Build (F7) 按钮，确保 0 Error(s), 0 Warning(s)

# 3. 烧录固件
# 连接 ST-Link 至 SWD接口 (PA13/SWDIO, PA14/SWCLK)
# 点击 Download (F8) 按钮
# 或使用 ST-Link Utility 烧录 01_BMS_PRO.hex 文件
```

#### 网关端

```bash
# 1. 打开工程
# 使用 Keil μVision 打开 02_gateway/MDK-ARM/gateway.uvprojx

# 2. 编译工程
# 点击 Build (F7)

# 3. 烧录固件
# 连接 ST-Link
# 点击 Download (F8)
```

### 9.3 硬件接线

#### BMS主控端接线

| 功能      | STM32引脚 | 外设引脚             |
| --------- | --------- | -------------------- |
| I2C_SCL   | PB6       | BQ769 SCL / OLED SCL |
| I2C_SDA   | PB7       | BQ769 SDA / OLED SDA |
| USART1_TX | PA9       | USB-TTL RX           |
| USART1_RX | PA10      | USB-TTL TX           |
| CAN_TX    | PA12      | TJA1050 TX           |
| CAN_RX    | PA11      | TJA1050 RX           |
| 5V/GND    | -         | 外设供电             |

#### 网关端接线

| 功能      | STM32引脚 | 外设引脚   |
| --------- | --------- | ---------- |
| SPI_SCK   | PA5       | W5500 SCK  |
| SPI_MISO  | PA6       | W5500 MISO |
| SPI_MOSI  | PA7       | W5500 MOSI |
| SPI_CS    | PA4       | W5500 CS   |
| W5500_INT | PA3       | W5500 INT  |
| W5500_RST | PA2       | W5500 RST  |
| CAN_TX    | PA12      | TJA1050 TX |
| CAN_RX    | PA11      | TJA1050 RX |
| USART1_TX | PA9       | USB-TTL RX |
| USART1_RX | PA10      | USB-TTL TX |

### 9.4 首次运行

1. **连接串口调试助手** (波特率 115200, 8N1)
2. **上电BMS主控端**，观察OLED是否显示电池参数
3. **上电网关端**，观察串口输出网络连接状态
4. **配置MQTT Broker地址** (在 `Int_MQTT_Client.c` 中修改)
5. **使用 MQTT.fx 或 mosquitto_sub 订阅** `bms/001/data` 查看数据

---

## 10. API参考

### 10.1 BMS主控端 API

#### 电池管理接口 (App_Battery.h)

```c
/**
 * @brief  初始化电池管理系统
 * @param  None
 * @retval 0: 成功, 其他: 错误码
 */
int App_Battery_Init(void);

/**
 * @brief  周期性电池数据采集任务
 * @param  None
 * @retval None
 * @note   建议在FreeRTOS任务中每100ms调用一次
 */
void App_Battery_Task(void);

/**
 * @brief  获取电池组总电压
 * @param  None
 * @retval 电压值，单位: mV
 */
uint32_t App_Battery_GetPackVoltage(void);

/**
 * @brief  获取充放电电流
 * @param  None
 * @retval 电流值，单位: mA (正值充电, 负值放电)
 */
int32_t App_Battery_GetCurrent(void);

/**
 * @brief  获取SOC
 * @param  None
 * @retval SOC百分比，范围 0~100
 */
uint8_t App_Battery_GetSOC(void);

/**
 * @brief  获取电芯温度
 * @param  index: 温度传感器索引 (0~2)
 * @retval 温度值，单位: ℃
 */
int8_t App_Battery_GetTemperature(uint8_t index);

/**
 * @brief  获取保护状态
 * @param  None
 * @retval 位图: bit0=OVP, bit1=UVP, bit2=OCP, bit3=OTP
 */
uint8_t App_Battery_GetProtectStatus(void);
```

#### BQ769驱动接口 (Int_BQ769.h)

```c
/**
 * @brief  初始化BQ769 AFE芯片
 * @param  None
 * @retval 0: 成功
 */
int Int_BQ769_Init(void);

/**
 * @brief  读取指定电芯电压
 * @param  cell: 电芯编号 (1~16)
 * @retval 电压值，单位: mV
 */
uint16_t Int_BQ769_ReadCellVoltage(uint8_t cell);

/**
 * @brief  读取电池组总电压
 * @param  None
 * @retval 电压值，单位: mV
 */
uint32_t Int_BQ769_ReadPackVoltage(void);

/**
 * @brief  读取电流 (库仑计数)
 * @param  None
 * @retval 电流值，单位: mA
 */
int32_t Int_BQ769_ReadCurrent(void);

/**
 * @brief  读取温度
 * @param  ts: 温度传感器编号 (1~3)
 * @retval 温度值，单位: 0.1K (需转换)
 */
uint16_t Int_BQ769_ReadTemperature(uint8_t ts);

/**
 * @brief  设置保护阈值
 * @param  ov: 过压阈值 (mV)
 * @param  uv: 欠压阈值 (mV)
 * @param  ocd: 过流放电阈值 (mA)
 * @param  occ: 过流充电阈值 (mA)
 * @retval None
 */
void Int_BQ769_SetProtection(uint16_t ov, uint16_t uv, 
                             uint16_t ocd, uint16_t occ);
```

#### OLED显示接口 (Int_OLED.h)

```c
/**
 * @brief  初始化SSD1315 OLED显示屏
 * @param  None
 * @retval 0: 成功
 */
int Int_OLED_Init(void);

/**
 * @brief  清屏
 * @param  None
 * @retval None
 */
void Int_OLED_Clear(void);

/**
 * @brief  显示字符串
 * @param  x: 起始X坐标 (0~127)
 * @param  y: 起始Y坐标 (0~7, 页地址)
 * @param  str: 待显示字符串
 * @retval None
 */
void Int_OLED_ShowString(uint8_t x, uint8_t y, char *str);

/**
 * @brief  显示数字
 * @param  x: 起始X坐标
 * @param  y: 起始Y坐标
 * @param  num: 待显示数字
 * @param  len: 显示长度
 * @retval None
 */
void Int_OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len);
```

### 10.2 网关端 API

#### CAN接口 (Int_CAN.h)

```c
/**
 * @brief  初始化CAN总线
 * @param  None
 * @retval 0: 成功
 */
int Int_CAN_Init(void);

/**
 * @brief  发送CAN数据帧
 * @param  id: 标准帧ID (11-bit)
 * @param  data: 数据指针
 * @param  len: 数据长度 (0~8)
 * @retval 0: 成功
 */
int Int_CAN_Send(uint16_t id, uint8_t *data, uint8_t len);

/**
 * @brief  接收CAN数据帧 (非阻塞)
 * @param  id: 输出接收到的帧ID
 * @param  data: 输出数据缓冲区
 * @param  len: 输出数据长度
 * @retval 0: 收到数据, 其他: 无数据
 */
int Int_CAN_Receive(uint16_t *id, uint8_t *data, uint8_t *len);
```

#### MQTT客户端 (Int_MQTT_Client.h)

```c
/**
 * @brief  初始化MQTT客户端
 * @param  broker_ip: MQTT服务器IP地址
 * @param  port: 服务器端口
 * @param  client_id: 客户端标识
 * @retval 0: 成功
 */
int Int_MQTT_Init(uint8_t *broker_ip, uint16_t port, char *client_id);

/**
 * @brief  连接MQTT服务器
 * @param  None
 * @retval 0: 连接成功
 */
int Int_MQTT_Connect(void);

/**
 * @brief  发布消息
 * @param  topic: 主题
 * @param  payload: 消息内容
 * @param  qos: 服务质量 (0/1)
 * @retval 0: 成功
 */
int Int_MQTT_Publish(char *topic, char *payload, uint8_t qos);

/**
 * @brief  订阅主题
 * @param  topic: 主题
 * @param  qos: 服务质量
 * @retval 0: 成功
 */
int Int_MQTT_Subscribe(char *topic, uint8_t qos);

/**
 * @brief  轮询接收MQTT消息
 * @param  None
 * @retval 0: 收到消息
 */
int Int_MQTT_Loop(void);
```

#### Modbus从站 (Int_Modbus.h)

```c
/**
 * @brief  初始化Modbus RTU从站
 * @param  slave_addr: 从机地址
 * @param  baudrate: 波特率
 * @retval 0: 成功
 */
int Int_Modbus_Init(uint8_t slave_addr, uint32_t baudrate);

/**
 * @brief  设置保持寄存器值
 * @param  addr: 寄存器地址 (40001-based)
 * @param  value: 寄存器值
 * @retval None
 */
void Int_Modbus_SetHoldingRegister(uint16_t addr, uint16_t value);

/**
 * @brief  轮询处理Modbus请求
 * @param  None
 * @retval None
 * @note   需在FreeRTOS任务中周期性调用
 */
void Int_Modbus_Poll(void);
```

---

## 11. 开发计划

### 当前版本 (v1.0)

- [x] BQ769 AFE基础通信与数据采集
- [x] OLED本地显示
- [x] CAN总线数据发送
- [x] 网关CAN数据接收
- [x] W5500以太网TCP连接
- [x] MQTT基础发布功能
- [x] Modbus RTU从站
- [x] FreeRTOS多任务调度

### 后续版本规划

| 版本 | 计划内容                    | 状态       |
| ---- | --------------------------- | ---------- |
| v1.1 | 增加主动均衡控制功能        | 🔜 计划中   |
| v1.2 | 实现SOH (健康状态) 估算     | 🔜 计划中   |
| v1.3 | 增加OTA远程固件升级         | 🔜 计划中   |
| v1.4 | 支持多BMS主控并联 (CAN组网) | 🔜 计划中   |
| v1.5 | 增加蓝牙BLE本地调试接口     | 🔜 计划中   |
| v2.0 | 支持ISO 26262功能安全标准   | 📋 远期规划 |

---

## 12. 常见问题

### Q1: BQ769通信失败，I2C无响应

**可能原因**:
- BQ769未正确供电 (需3.3V和电池组供电)
- I2C地址配置错误
- 上拉电阻缺失 (建议4.7kΩ)

**解决方法**:
1. 检查BQ769的REGOUT引脚是否有3.3V输出
2. 使用示波器检查I2C波形
3. 确认 `Int_BQ769_BSP.h` 中地址宏定义正确

### Q2: OLED显示花屏或无显示

**可能原因**:
- I2C地址错误 (SSD1315常见地址0x78或0x7A)
- 初始化序列不正确
- 供电不足

**解决方法**:
1. 检查OLED模块背面电阻配置的I2C地址
2. 确认初始化代码与SSD1315 datasheet一致
3. 确保供电电压为3.3V且电流充足

### Q3: W5500无法连接网络

**可能原因**:
- SPI接线错误
- 网络参数配置错误
- 路由器/防火墙限制

**解决方法**:
1. 检查SPI接线 (SCK/MISO/MOSI/CS)
2. 确认 `wizchip_conf.c` 中MAC地址、IP地址、网关配置正确
3. 使用ping测试网络连通性
4. 检查MQTT Broker是否允许匿名连接

### Q4: CAN总线无数据

**可能原因**:
- 波特率不匹配
- 终端电阻缺失 (CAN_H和CAN_L之间需120Ω)
- 收发器未供电

**解决方法**:
1. 确认两端波特率一致 (默认500kbps)
2. 在CAN总线两端各添加120Ω终端电阻
3. 检查TJA1050的VCC供电 (5V)

### Q5: FreeRTOS任务栈溢出

**可能原因**:
- 任务栈空间分配不足
- 中断嵌套过深
- 递归调用或大量局部变量

**解决方法**:
1. 增大任务栈大小 (xTaskCreate的usStackDepth参数)
2. 启用 `configCHECK_FOR_STACK_OVERFLOW = 2`
3. 实现 `vApplicationStackOverflowHook` 调试钩子

---

## 附录

### A. 参考资料

- [BQ769x2 数据手册](https://www.ti.com/lit/ds/symlink/bq76920.pdf) — Texas Instruments
- [STM32F103xx 参考手册](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) — ST
- [FreeRTOS 官方文档](https://www.freertos.org/Documentation/RTOS_book.html) — Amazon
- [W5500 数据手册](https://docs.wiznet.io/Product/iEthernet/W5500) — WizNet
- [MQTT v3.1.1 规范](https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html) — OASIS

### B. 相关工具

- [MQTT.fx](https://mqttfx.jensd.de/) — MQTT客户端调试工具
- [Modbus Poll](https://www.modbustools.com/modbus_poll.html) — Modbus主站调试工具
- [CANalyzer](https://www.vector.com/int/en/products/products-a-z/software/canalyzer/) / [PCAN-View](https://www.peak-system.com/PCAN-View.242.0.html) — CAN总线分析工具
- [Wireshark](https://www.wireshark.org/) — 网络抓包分析

### C. 许可证

本项目为**尚硅谷**教学项目，代码仅供学习参考使用。

第三方库许可证:
- FreeRTOS: MIT License
- cJSON: MIT License
- MQTTPacket: Eclipse Public License v1.0
- STM32 HAL / CMSIS: BSD-3-Clause

---

*本文档最后更新于: 2026年7月28日*
*项目维护: At-SiliconValley*
