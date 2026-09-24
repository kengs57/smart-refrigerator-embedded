```markdown
# 🧊 IoT Smart Refrigerator (Hardware & Embedded Control System)

> **인공지능 비전 센서(HuskyLens) 기반 식료품 자동 인식, 멀티 앵글 ESP32-CAM 실시간 모니터링 및 MQTT 원격 제어를 구현한 스마트 냉장고 임베디드 시스템**

---

## 📖 프로젝트 개요

냉장고 문을 열지 않고도 내부 식료품의 적재 현황과 온습도 상태를 실시간으로 모니터링하고, 내부 조명 및 보조 기기를 원격 제어할 수 있는 스마트 냉장고 프로토타입입니다.

본 저장소는 시스템의 핵심 축인 **임베디드 펌웨어(Arduino, ESP8266, ESP32-CAM) 및 전원/통신 하드웨어 설계** 소스 코드를 포함하고 있습니다.

---

## 📌 주요 담당 업무 및 기술적 해결 (Hardware & Embedded)

### 1. AI 객체 인식 및 데이터 패킷화 (`huskylens/`)
* **HuskyLens I2C 인터페이스 구축:** DFRobot HuskyLens와 Arduino Uno 간 I2C 통신을 통해 사전에 학습된 식료품(5종)의 실시간 바운딩 박스 및 ID를 추출
* **노이즈 필터링 및 카운팅 알고리즘:** 0.5초 단위 샘플링 후, 10초 주기로 누적 인식 횟수(6회 이상)를 판별하여 순간적인 오인식을 방지하는 노이즈 필터링 로직 구현
* **패킷 포맷팅:** 판별된 5종의 유무 상태를 구분자(`|`) 기반의 문자열 데이터 패킷(`1|0|1|0|1`)으로 구조화하여 SoftwareSerial(115200bps)을 통해 게이트웨이로 안정적 송신

### 2. IoT 게이트웨이 및 원격 릴레이 제어 (`esp8266_gateway/`)
* **센서 데이터 통합 및 MQTT 통신:** 
  * AM2302(DHT22) 센서를 이용해 냉장고 내부 온습도를 2초 주기로 수집
  * PubSubClient 라이브러리를 활용해 MQTT 브로커와 통신 (`jb202211/temp`, `jb202211/humi`, `jb202211/food`)
* **4채널 릴레이 액추에이터 제어:** 
  * Node-RED 대시보드로부터 수신되는 제어 명령(`ona`, `offa` 등)을 파싱하여 4개 릴레이 채널을 Active Low 방식으로 제어
* **소프트웨어 안정성:** 네트워크 단절 발생 시 자동 재접속(`reconnect()`) 및 `millis()` 기반 비동기 타이머 로직 설계

### 3. 실시간 내부 멀티 앵글 스트리밍 (`esp32_cam/`)
* **4채널 ESP32-CAM HTTP 웹 서버:** 각 선반과 수납공간을 비추는 4대의 ESP32-CAM 모듈을 개별 네트워크 노드로 구성
* **스트리밍 최적화:** CIF 해상도 적용 및 좌우 미러링(`set_hmirror`) 설정을 통해 네트워크 대역폭 부담을 줄이고 실시간 MJPEG 스트림 전송 지연 최소화

### 4. 하드웨어 회로 및 전원 분배 설계
* **다중 전원 분배 보드 자체 제작:** 다수의 ESP32-CAM(5V)과 고휘도 LED 바(12V) 동시 구동 시 발생하는 MCU의 전류 공급 한계(핀당 최대 40mA) 문제를 해결하기 위해, 별도 외부 어댑터 인가용 전원 분배 만능기판(PCB)을 직접 설계 및 납땜
* **220V 멀티탭 배선 개조:** 안전 접지 라인을 유지하면서 내부 개별 스위치 접점을 4채널 릴레이 모듈과 직렬 연결하여 상용 전원 기기를 안전하게 원격 제어할 수 있도록 물리 배선 개조

---

## 🛠️ System Architecture

```text
┌─────────────────┐       I2C       ┌──────────────┐
│    HuskyLens    │ ──────────────> │ Arduino Uno  │
│  (AI 비전 센서)  │                 │ (연산/필터링)  │
└─────────────────┘                 └──────┬───────┘
                                           │ SoftwareSerial (115200bps)
                                           ▼
┌─────────────────┐      DHT22      ┌──────────────┐       MQTT        ┌──────────────────────┐
│  DHT22 (AM2302) │ ──────────────> │   ESP8266    │ <───────────────> │     MQTT Broker      │
│   (온습도 센서)  │                 │  (Wemos D1)  │  (Publish/Sub)    │ (broker.mqtt-dash..) │
└─────────────────┘                 └──────┬───────┘                   └──────────┬───────────┘
                                           │ GPIO Output                          │
                                           ▼                                      ▼
                                    ┌──────────────┐                   ┌──────────────────────┐
                                    │ 4채널 릴레이   │                   │  Node-RED Dashboard  │
                                    │ (멀티탭 전원) │                   │  (클라우드 VM 관제)   │
                                    └──────────────┘                   └──────────────────────┘

┌─────────────────┐   HTTP MJPEG Stream (포트포워딩 / DDNS)
│  ESP32-CAM x 4  │ ─────────────────────────────────────────────────────────────┘
└─────────────────┘

```

---

## 📂 디렉터리 구조

```text
smart-refrigerator-embedded/
│
├── README.md
├── .gitignore
│
├── esp32_cam/                      # [모듈 1] ESP32-CAM 4채널 실시간 영상 스트리밍
│   └── CameraWebServer/
│       ├── CameraWebServer.ino     # 카메라 서버 메인 펌웨어
│       ├── app_httpd.cpp           # HTTP 핸들러 및 MJPEG 스트리밍 구현
│       ├── camera_pins.h           # AI-Thinker GPIO 핀 매핑
│       └── camera_index.h          # 뷰어 웹 UI 데이터 (HTML gzip)
│
├── huskylens/                      # [모듈 2] HuskyLens AI 객체 인식 및 패킷화
│   └── huskyi2c/
│       └── huskyi2c.ino            # 아두이노 우노 메인 펌웨어 (10초 주기 필터링 로직)
│
└── esp8266_gateway/                # [모듈 3] 온습도 수집, 릴레이 제어 및 MQTT 게이트웨이
    └── gateway/
        └── gateway.ino             # Wemos D1 메인 펌웨어 (MQTT 통신 & 시리얼 수신)

```

---

## ⚙️ 하드웨어 구성 사양 (Hardware Components)

| 분류 | 부품명 | 주요 사양 및 용도 |
| --- | --- | --- |
| **Main Controller** | Arduino Uno R3 | 허스키렌즈 데이터 연산 및 1차 노이즈 필터링 |
| **IoT Gateway** | Wemos D1 R1 (ESP8266) | Wi-Fi 연결, MQTT 통신, 온습도 측정, 릴레이 제어 |
| **Camera Module** | AI-Thinker ESP32-CAM (x4) | 실시간 MJPEG 비디오 스트리밍 (CIF 해상도) |
| **AI Vision Sensor** | DFRobot HuskyLens | 5종 식료품 객체 인식 및 I2C 통신 |
| **Environment Sensor** | AM2302 (DHT22) | 냉장고 내부 온도 및 습도 모니터링 |
| **Actuator** | 4-Channel Relay Module | 멀티탭 전원 및 조명 On/Off 제어 (220V 제어) |
| **Lighting** | 12V LED Bar (x2) | 카메라 인식을 위한 냉장고 내부 조명 |
| **Power Distribution** | 커스텀 배선 보드 & 어댑터 | 12V/2A, 5V 어댑터 및 자체 제작 전원 분배 PCB |

---

## 🚀 시작하기 (Setup & Upload)

### 1. 개발 환경 및 필수 라이브러리

* **IDE:** Arduino IDE 1.8.x 이상
* **필수 라이브러리:**
* `PubSubClient` (by Nick O'Leary)
* `DHT sensor library` (by Adafruit)
* `HUSKYLENS` (by DFRobot)
* `ESP8266 Board Package` / `ESP32 Board Package`



### 2. 펌웨어 설정 (Wi-Fi 및 네트워크)

* 보안을 위해 소스 코드 내 Wi-Fi 정보 및 브로커 주소는 환경에 맞게 입력 후 업로드해야 합니다.
```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "YOUR_MQTT_BROKER_IP";

```



### 3. 보드별 업로드 타겟

* `huskylens/huskyi2c/huskyi2c.ino` ➔ **Arduino Uno**
* `esp8266_gateway/gateway/gateway.ino` ➔ **LOLIN(WEMOS) D1 R1**
* `esp32_cam/CameraWebServer/CameraWebServer.ino` ➔ **AI Thinker ESP32-CAM** (PSRAM 활성화)

```

