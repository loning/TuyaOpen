# 双向音视频流终端方案调研报告

> 日期：2026-04-04  
> 目标：寻找适合做双向音视频流传输终端的方案，设备端只负责采集和播放，AI 推理在服务端完成。  
> 最终推荐：**Waveshare ESP32-P4-NANO 开发板**

---

## 一、需求分析

### 核心需求

- 双向实时音频流（麦克风输入 + 扬声器输出）
- 双向实时视频流（摄像头输入 + 屏幕输出）
- 设备端仅做采集/播放，AI 推理在云端/服务端
- 低延迟（<500ms，理想 <200ms）

### 期望特性

- 启动快（秒级），即插即用
- 运行稳定（可 7×24 小时运行）
- 开放系统，不绑定特定云平台
- 便于接入 Home Assistant
- 最好有一体化硬件（屏幕+摄像头+麦克风+扬声器）
- 有线网络支持（WiFi 在嵌入式场景下不够稳定）

---

## 二、流媒体协议选型

| 协议 | 延迟 | 方向 | 适合场景 |
|------|------|------|----------|
| **WebRTC** | <200ms | **双向** | 视频通话、实时互动 |
| RTSP | 1-3 秒 | 通常单向 | 监控摄像头 |
| RTMP | 1-3 秒 | 通常单向 | 直播推流 |
| HLS | 5-30 秒 | 单向 | 点播/直播回放 |

### WebRTC 核心优势

- 浏览器原生支持（Chrome/Safari/Firefox），无需插件
- 点对点（P2P）直连，延迟最低
- 自带音视频编解码（VP8/VP9/H.264 + Opus）
- 自带回声消除、降噪、自动增益
- 自带网络自适应，丢包/带宽变化自动调码率

**结论：WebRTC 是双向音视频流的最佳协议。**

---

## 三、平台评估

### 3.1 TuyaOpen / T5AI

TuyaOpen 是涂鸦的开源 AIoT 开发 SDK，多平台抽象层架构：

```
应用层（AI Agent、语音、云连接）
        ↓
TuyaOpen 抽象层（统一 API：tal_* / tkl_*）
        ↓
平台适配层（ESP-IDF / BK SDK / T5 SDK / Linux）
```

**支持的芯片平台：**

| 平台 | 芯片 | 来源 |
|------|------|------|
| Tuya T2 | BK7231N (博通集成) | 涂鸦自有模组 |
| Tuya T3 | BK7231 系列 | 涂鸦自有模组 |
| Tuya T5/T5AI | ARM 核 | 涂鸦自研 |
| ESP32/C3/S3 | 乐鑫 ESP 系列 | 第三方支持 |
| BK7231N | 博通集成 | 第三方支持 |
| LN882H | 上海灵动 | 第三方支持 |
| Ubuntu/Linux | x86/ARM | 仿真/调试 |

**AI 功能与云依赖：**

TuyaOpen SDK 的 AI 功能（语音识别、LLM 推理、TTS）**强依赖涂鸦云**：

- `ai_agent_init()` 在 MQTT 连接涂鸦云成功后才触发
- 语音/LLM 推理全部在云端完成
- 必须注册涂鸦 IoT 平台、购买 license（UUID + AuthKey）

但 TuyaOpen SDK 的**基础设施层（驱动、网络、TLS、OS 抽象）可以独立使用**，不必接涂鸦云。项目中的 mimiclaw 应用就是实例——使用 `tal_*`/`tkl_*` 层，AI 部分直接对接 DeepSeek/Claude/GPT 等第三方 LLM。

**T5AI 做双向音视频终端的局限：**

- 底层 BSP/HAL 目前**只有 TuyaOpen SDK 提供**，无法选择其他开发框架
- 项目内已有 P2P/WebRTC 相关代码（`src/tuya_p2p/`，含 ICE、RTP、PJSIP），但文档和示例不足
- 生态封闭，社区资源少，遇到问题排查困难

### 3.2 ESP32 系列

**芯片家族：**

| 型号 | 核心 | WiFi | BLE | 特色 | 模组价格 |
|------|------|------|-----|------|---------|
| ESP32 | 双核 Xtensa LX6 @240MHz | 2.4G | 4.2 | 经典款 | ~¥10 |
| ESP32-S2 | 单核 Xtensa LX7 @240MHz | 2.4G | 无 | USB OTG | ~¥8 |
| ESP32-S3 | 双核 Xtensa LX7 @240MHz | 2.4G | 5.0 | AI 加速(SIMD) | ~¥12 |
| ESP32-C3 | 单核 RISC-V @160MHz | 2.4G | 5.0 | 最便宜 | ~¥6 |
| ESP32-C6 | 单核 RISC-V @160MHz | 2.4G + WiFi 6 | 5.0 + Thread/Zigbee | Matter 首选 | ~¥10 |
| ESP32-H2 | 单核 RISC-V @96MHz | 无 | 5.0 + Thread/Zigbee | 纯低功耗 | ~¥6 |
| **ESP32-P4** | **双核 RISC-V @400MHz** | **无(需外挂)** | **无** | **硬件 H.264，AI/HMI** | **~¥20** |
| ESP32-C5 | RISC-V | 2.4G + 5GHz | 5.0 + Thread | 首款双频 WiFi | 新品 |

**开发框架生态（10+ 种）：** ESP-IDF、Arduino、ESPHome、MicroPython、CircuitPython、Zephyr、NuttX、Rust (esp-hal)、Tasmota、WLED 等。

**Home Assistant 集成（最强平台）：**

- **ESPHome**: YAML 配置，OTA 刷固件，HA 自动发现
- **Tasmota**: 刷固件，Web 界面配置，MQTT 自动发现
- **Matter**: ESP32-C6/H2 原生支持
- **Bluetooth Proxy**: ESP32 当 BLE 代理
- **Voice Assistant**: ESP32-S3 跑本地唤醒词 + HA 语音管道

**ESP32 WiFi 网络的局限：**

- 仅 2.4GHz（C5 之前），干扰严重
- WiFi 吞吐低（实测 TCP 5-15 Mbps）
- PCB 天线穿墙能力差，连接偶尔断线
- WiFi 功耗 100-200mA，电池不友好
- lwIP 栈并发受限，大量数据传输易出问题
- **不适合对网络要求高的音视频流场景**

**关键结论：ESP32-P4 通过硬件 H.264 编码和有线以太网接口解决了上述痛点。**

### 3.3 Luckfox Pico (RV1106)

| 优势 | 劣势 |
|------|------|
| 启动快（2-3 秒） | 双向音视频流方案**不成熟** |
| 硬件 ISP + 视频编解码 | 双向音频无现成开源实现 |
| 体积小、功耗低 | GStreamer 硬件编码不支持 RV1106 |
| ¥70-100 | 只能用瑞芯微私有 rockit 库 |

现有开源资源均不完整：

| 项目 | 能力 |
|------|------|
| [luckfox_pico_rkmpi_example](https://github.com/LuckfoxTECH/luckfox_pico_rkmpi_example) | 单向 RTSP 视频，无音频 |
| [JetKVM](https://github.com/jetkvm/kvm) | Pion WebRTC 视频，无音频 |
| [OpenIPC](https://github.com/OpenIPC/firmware) | RTSP 视频可用，双向音频开发中 |

**结论：RV1106 芯片能力足够，但软件生态还没跟上，做双向音视频需要自己填坑太多。**

### 3.4 树莓派

| 优势 | 劣势 |
|------|------|
| 生态最成熟，方案最多 | 启动慢（20-30 秒） |
| 千兆以太网，网络能力强 | 功耗相对高（3-5W） |
| go2rtc 原生支持 HA | 需要自行组装外设 |
| 可同时跑 Home Assistant | 偶尔需要系统维护 |

**结论：方案成熟但启动慢，适合常开不关机场景，不如 MCU 方案即插即用。**

### 3.5 其他方案

| 方案 | 评价 |
|------|------|
| 二手 iPad + WebRTC 浏览器 | 硬件最强但系统封闭 |
| Android 平板 | 长时间运行不稳定，卡顿 |
| 消费级带屏设备（Echo Show/小度） | 系统封闭，接自有 AI 后端困难 |
| Realtek AMB82-MINI | 硬件 H.264/H.265，有 KVS 示例，但生态不如乐鑫 |
| Vivid Unit 一体机 | 自带屏+麦+扬声器，但价格高(~¥800)、启动慢 |

---

## 四、ESP32-P4 深入分析

### 4.1 芯片核心规格

| 参数 | 规格 |
|------|------|
| CPU | 双核 RISC-V 32-bit @400MHz + 低功耗核 @40MHz |
| DSP | DSP 扩展 + 单精度 FPU + AI 指令扩展 |
| 内存 | 768KB SRAM + 8KB TCM（支持外挂 PSRAM 最大 32MB） |
| 视频编码 | **硬件 H.264，1080p@30fps** |
| 图像处理 | 集成 ISP + JPEG codec + 2D GPU |
| 摄像头接口 | MIPI-CSI 2-lane |
| 显示接口 | MIPI-DSI 2-lane |
| USB | USB OTG 2.0 HS |
| 以太网 | 内置 MAC（需外接 PHY） |
| 音频 | I2S 接口 |
| 安全 | Secure Boot、Flash 加密、AES/SHA/RSA 硬件加速 |
| WiFi/BLE | **无**（需外挂 ESP32-C5/C6） |

### 4.2 官方 WebRTC 方案

乐鑫官方提供 [esp-webrtc-solution](https://github.com/espressif/esp-webrtc-solution)，包含 12 个示例：

| 示例 | 功能 |
|------|------|
| **doorbell_demo** | **双向音视频**，门铃场景 |
| **videocall_demo** | **视频通话** |
| Amazon KVS 集成 | AWS Kinesis Video Streams |
| **OpenAI Realtime** | 实时语音对话 |
| Janus 网关 | 标准 WebRTC 网关对接 |
| WHIP 推流 | WebRTC-HTTP Ingestion Protocol |

组件架构：
- `esp_webrtc` — WebRTC 核心协议栈
- `esp_peer` — PeerConnection 管理
- `esp_capture` — 媒体采集（摄像头+麦克风）
- `av_render` — 音视频播放

### 4.3 带网口的 ESP32-P4 开发板

| 开发板 | WiFi | 以太网 | PoE | 尺寸 | 参考价 | 特点 |
|--------|------|--------|-----|------|--------|------|
| **[ESP32-P4-NANO](https://www.waveshare.net/wiki/ESP32-P4-Nano-StartPage)** | ✅ ESP32-C6 (WiFi 6 + BLE 5.4) | ✅ 100M RJ45 | ✅ 可选 | 50×50mm | ~¥140 | **有线+无线双备份，最紧凑** |
| [ESP32-P4-ETH](https://www.waveshare.com/esp32-p4-eth.htm) | ❌ 无 | ✅ 100M RJ45 | ✅ 支持 | - | ~¥80-100 | 纯有线，最精简最便宜 |
| [ESP32-P4-Module-DEV-KIT](https://www.waveshare.com/esp32-p4-module-dev-kit.htm) | ✅ ESP32-C6 (WiFi 6 + BLE 5) | ✅ 100M RJ45 | ✅ 预留 | 信用卡大小 | ~¥160 | 4×USB，40pin 兼容树莓派 HAT |
| [ESP32-P4-Function-EV-Board](https://docs.espressif.com/projects/esp-dev-kits/zh_CN/latest/esp32p4/) | ✅ 子板 | ✅ 100M RJ45 | ❌ | 较大 | ~¥300+ | 乐鑫官方评估板 |
| [DFRobot FireBeetle 2 ESP32-P4](https://wiki.dfrobot.com.cn/SKU_DFR1172_FireBeetle_2_ESP32_P4) | ✅ | 待确认 | 待确认 | - | 待确认 | DFRobot 出品 |

所有板子的以太网架构相同：**ESP32-P4 内置 MAC → RMII → IP101GRI PHY → 网络变压器 → RJ45**，100Mbps 有线连接。

---

## 五、开源 WebRTC 库（嵌入式可用）

| 项目 | 语言 | 平台 | 特点 |
|------|------|------|------|
| [esp-webrtc-solution](https://github.com/espressif/esp-webrtc-solution) | C | ESP32-P4/S3 | 乐鑫官方，双向音视频，集成 KVS/Janus/WHIP/OpenAI |
| [libpeer](https://github.com/sepfy/libpeer) | C | ESP32/树莓派 | 极简 WebRTC（~1.5k stars），H264 + Opus + G.711，BSD 协议 |
| [go2rtc](https://github.com/AlexxIT/go2rtc) | Go | Linux/Docker | 万能流媒体网关，WebRTC/RTSP 互转，**支持涂鸦设备双向音频**，HA 集成 |
| [Amazon KVS WebRTC SDK](https://github.com/awslabs/amazon-kinesis-video-streams-webrtc-sdk-c) | C | 通用 | AWS 官方，双向音视频，AWS 管理信令和 TURN |
| [Nabto Edge Device WebRTC](https://github.com/nabto/edge-device-webrtc) | C++ | Linux/嵌入式 | P2P 直连，无需端口转发，内置 IAM + OAuth |
| [GStreamer webrtcbin](https://gstreamer.freedesktop.org/documentation/webrtc/index.html) | C | Linux 嵌入式 | 管道式架构，Jetson/i.MX6/RK3228H 验证过 |

---

## 六、全方案对比

| 方案 | 双向音视频 | 启动速度 | 网络稳定性 | HA 集成 | 开放程度 | 成本 |
|------|-----------|---------|-----------|---------|---------|------|
| **ESP32-P4-NANO** | ✅ 官方示例 | ⭐⭐⭐ 秒启动 | ⭐⭐⭐ 有线+WiFi | ⭐⭐ 需桥接 | ⭐⭐⭐ 完全开放 | ¥140-700 |
| ESP32-P4-ETH | ✅ 官方示例 | ⭐⭐⭐ 秒启动 | ⭐⭐⭐ 有线 | ⭐⭐ 需桥接 | ⭐⭐⭐ 完全开放 | ¥80-500 |
| ESP32-P4-Module-DEV-KIT | ✅ 官方示例 | ⭐⭐⭐ 秒启动 | ⭐⭐⭐ 有线+WiFi | ⭐⭐ 需桥接 | ⭐⭐⭐ 完全开放 | ¥160-700 |
| Realtek AMB82-MINI | ✅ KVS 示例 | ⭐⭐⭐ 秒启动 | ⭐⭐ 仅 WiFi | ⭐ 需自行实现 | ⭐⭐ Arduino | ~¥250 |
| 树莓派 + go2rtc | ✅ 成熟 | ⭐ 20-30 秒 | ⭐⭐⭐ 千兆有线 | ⭐⭐⭐ 原生 | ⭐⭐⭐ 完全开放 | ¥400-700 |
| Vivid Unit 一体机 | ✅ GStreamer | ⭐ 20-30 秒 | ⭐⭐⭐ 有线 | ⭐⭐ 可集成 | ⭐⭐⭐ Linux | ~¥800 |
| T5AI + TuyaOpen | ⚠️ 代码不完善 | ⭐⭐⭐ 秒启动 | ⭐⭐ 仅 WiFi | ⭐ LocalTuya | ⭐ 绑涂鸦 SDK | ¥200+ |
| Luckfox Pico | ❌ 需自行开发 | ⭐⭐⭐ 2-3 秒 | ⭐⭐ 有线可选 | ⭐ 需自行实现 | ⭐⭐ Linux | ¥70-100 |
| 二手 iPad | ✅ 浏览器 WebRTC | ⭐⭐ 常开不关 | ⭐⭐ 仅 WiFi | ⭐⭐ Companion | ⭐ 封闭系统 | ¥500-800 |

---

## 七、最终推荐：Waveshare ESP32-P4-NANO

### 推荐理由

1. **有线 + 无线双网络**：100M 以太网保证音视频流稳定，WiFi 6 作为备份或灵活部署，PoE 供电一根线搞定
2. **硬件 H.264 编码 1080p@30fps**：解决了 ESP32 系列一直以来视频能力不足的问题
3. **官方 WebRTC 方案成熟**：esp-webrtc-solution 提供 doorbell_demo（双向音视频）、videocall_demo、OpenAI Realtime 对接等 12 个示例
4. **RTOS 秒级启动**：即插即用，不像 Linux 方案要等 20-30 秒
5. **接口齐全**：MIPI-CSI 摄像头（兼容树莓派摄像头）、MIPI-DSI 屏幕、板载麦克风、扬声器接口
6. **不绑定云平台**：可自由对接任何 AI 后端（OpenAI、DeepSeek、Claude、自建服务等）
7. **生态开放**：乐鑫 ESP-IDF 生态，文档完善，社区活跃
8. **体积小巧**：50×50mm，适合做成桌面终端产品
9. **成本可控**：裸板 ~¥140，全套（板+摄像头+屏幕+PoE）~¥700

### 对比其他 ESP32-P4 开发板的优势

| 对比 | ESP32-P4-NANO 赢在哪 |
|------|----------------------|
| vs ESP32-P4-ETH | 多了 WiFi 6 + BLE 5.4，部署更灵活 |
| vs ESP32-P4-Module-DEV-KIT | 更小巧（50×50mm），价格更低 |
| vs ESP32-P4-Function-EV-Board | 第三方板，更便宜，集成度更高 |
| vs T5AI | 不绑涂鸦云，WebRTC 官方支持，生态开放 |
| vs 树莓派 | 秒启动，功耗低，体积小 |
| vs Luckfox Pico | WebRTC 方案成熟，不用自己造轮子 |

### 需要注意

- ESP32-P4-NANO 是**微雪(Waveshare)第三方产品**，非乐鑫官方开发板
- 以太网为 100Mbps（非千兆），但对于 1080p WebRTC 流绰绰有余
- 需要外接摄像头模组（兼容树莓派摄像头，便宜好买）
- 需要外接屏幕（MIPI-DSI，可选配 10.1 寸 1280×800 触屏）
- 需要外接扬声器

### 套餐选择建议

| 套餐 | 包含 | 价格 | 建议 |
|------|------|------|------|
| 裸板 | ESP32-P4-NANO 主板 | ~¥140 | 已有外设可选 |
| 带摄像头 | 主板 + CSI 摄像头 | ~¥200 | **验证推荐，最小可用** |
| 带摄像头+屏幕+PoE | 主板 + 摄像头 + 屏幕 + PoE 模块 | ~¥700 | 完整终端 |
| 全套（10.1 寸屏） | 主板 + 摄像头 + 10.1 寸触屏 + PoE | ~¥800 | 桌面终端完整形态 |

---

## 八、系统架构设计

基于 ESP32-P4-NANO 的双向音视频 AI 终端架构：

```
┌─────────────────────────────────────────────────┐
│                 AI 服务端                         │
│  ┌──────────┐  ┌──────────┐  ┌───────────────┐  │
│  │ LLM 推理  │  │ ASR/TTS  │  │ 视觉理解/生成  │  │
│  └──────────┘  └──────────┘  └───────────────┘  │
└────────────────────┬────────────────────────────┘
                     │ WebRTC (双向音视频流)
                     │ <200ms 延迟
                     │
        ┌────────────┴────────────┐
        │ 有线以太网(100M) / WiFi 6 │
        └────────────┬────────────┘
                     │
┌────────────────────┴────────────────────────────┐
│            ESP32-P4-NANO 终端设备                 │
│                                                   │
│  ┌─────────┐  ┌────────────┐  ┌──────────────┐  │
│  │ CSI 摄像头│  │ 板载麦克风  │  │ DSI 触摸屏    │  │
│  │ (输入)    │  │ (输入)      │  │ (输出+交互)   │  │
│  └─────────┘  └────────────┘  └──────────────┘  │
│                                                   │
│  ┌────────────────────────────────────────────┐  │
│  │ esp-webrtc-solution                         │  │
│  │ ┌──────────┐ ┌─────────┐ ┌──────────────┐ │  │
│  │ │H.264 硬编 │ │Opus 音频│ │ WebRTC 协议栈 │ │  │
│  │ └──────────┘ └─────────┘ └──────────────┘ │  │
│  └────────────────────────────────────────────┘  │
│                                                   │
│  ┌──────────┐  ┌────────────┐  ┌──────────────┐ │
│  │ 扬声器    │  │ PoE 供电    │  │ ESP32-C6     │ │
│  │ (输出)    │  │ (一线通)    │  │ (WiFi/BLE)   │ │
│  └──────────┘  └────────────┘  └──────────────┘ │
└──────────────────────────────────────────────────┘
```

### 数据流

1. **上行（设备→服务端）**：摄像头 → ISP → H.264 硬件编码 → WebRTC → 服务端；麦克风 → Opus 编码 → WebRTC → 服务端
2. **下行（服务端→设备）**：服务端 → WebRTC → H.264 解码 → DSI 屏幕显示；服务端 → WebRTC → Opus 解码 → 扬声器播放
3. **控制通道**：WebRTC DataChannel 或 MQTT 用于控制指令（Home Assistant 集成等）

---

## 九、下一步行动建议

### 第一阶段：验证（1-2 周）

1. **购买硬件**：ESP32-P4-NANO 带摄像头套餐（~¥200），验证最小可用
2. **搭建环境**：安装 ESP-IDF，编译 esp-webrtc-solution
3. **跑通 Demo**：运行 doorbell_demo，验证双向音视频效果
4. **网络测试**：重点测试有线以太网下的延迟和稳定性

### 第二阶段：对接 AI 后端（2-4 周）

1. **参考 OpenAI Realtime 示例**，对接自有或第三方 LLM 服务
2. **实现音频管道**：麦克风 → WebRTC → 服务端 ASR → LLM → TTS → WebRTC → 扬声器
3. **实现视频管道**：摄像头 → WebRTC → 服务端视觉理解 → 响应

### 第三阶段：集成与产品化（4-8 周）

1. **Home Assistant 集成**：通过 MQTT 或 go2rtc 桥接
2. **外壳设计**：3D 打印或亚克力外壳，集成屏幕+摄像头+扬声器
3. **稳定性优化**：看门狗、OTA 升级、异常恢复
4. **完善 UI**：触屏交互界面

### 备选路径

如验证阶段发现 ESP32-P4-NANO 的 100Mbps 以太网或 WiFi 性能不满足需求，转向**树莓派 5 + go2rtc** 方案（常开不关机，千兆以太网）。

---

## 附录 A：参考链接

### ESP32-P4 相关

- [esp-webrtc-solution](https://github.com/espressif/esp-webrtc-solution) — 乐鑫官方 WebRTC 方案
- [ESP32-P4 Matter Camera](https://developer.espressif.com/blog/2026/01/introducing-esp-matter-camera/) — ESP32-P4 摄像头方案
- [ESP32-P4-NANO Wiki](https://www.waveshare.net/wiki/ESP32-P4-Nano-StartPage) — 微雪官方文档
- [ESP32-P4-NANO 商城](https://www.waveshare.com/esp32-p4-nano.htm) — 购买链接
- [ESP32-P4-ETH](https://www.waveshare.com/esp32-p4-eth.htm) — 纯有线版本
- [ESP32-P4-Module-DEV-KIT](https://www.waveshare.com/esp32-p4-module-dev-kit.htm) — 信用卡大小版本
- [ESP32-P4-ETH 评测 (CNX Software)](https://www.cnx-software.com/2025/08/20/waveshare-esp32-p4-eth-development-board-supports-ethernet-and-poe/) — 第三方评测
- [DFRobot FireBeetle 2 ESP32-P4](https://wiki.dfrobot.com.cn/SKU_DFR1172_FireBeetle_2_ESP32_P4) — DFRobot 版本

### WebRTC 开源库

- [libpeer](https://github.com/sepfy/libpeer) — 极简嵌入式 WebRTC 库
- [go2rtc](https://github.com/AlexxIT/go2rtc) — 万能流媒体网关
- [Amazon KVS WebRTC C SDK](https://github.com/awslabs/amazon-kinesis-video-streams-webrtc-sdk-c) — AWS WebRTC SDK
- [Nabto Edge Device WebRTC](https://github.com/nabto/edge-device-webrtc) — P2P WebRTC 方案
- [GStreamer webrtcbin](https://gstreamer.freedesktop.org/documentation/webrtc/index.html) — GStreamer WebRTC 模块

### 其他平台

- [Realtek AMB82-MINI](https://www.seeedstudio.com/AMB82-MINI-RTL8735B-IoT-AI-Camera-Dev-Board-p-5584.html) — Realtek 开发板
- [Vivid Unit](https://www.uugear.com/product/vivid-unit/) — 一体化 Linux SBC
- [LuckfoxTECH/luckfox-pico](https://github.com/LuckfoxTECH/luckfox-pico) — Luckfox 官方 SDK
- [JetKVM](https://github.com/jetkvm/kvm) — RV1106 WebRTC 参考实现
- [OpenIPC](https://github.com/OpenIPC/firmware) — 开源 IP 摄像头固件

### TuyaOpen 相关

- [TuyaOpen SDK](https://github.com/tuya/tuyaopen) — 涂鸦开源 IoT SDK
- [TuyaOpen 文档](https://tuyaopen.ai/docs/about-tuyaopen) — 官方开发者文档
- [LocalTuya](https://github.com/rospogriern/localtuya) — Home Assistant 局域网涂鸦集成
