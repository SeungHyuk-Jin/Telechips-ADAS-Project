#  텔레칩스 부트캠프 프로젝트
주제 : 차량시스템을 위한 지능형 IoT 장치 개발 ->사고 예방 운전 보조 시스템 (ADAS Prototype)​

- 1차: STM32 + Raspberry Pi 기반 구현
- STM32: 센서 데이터 수집 및 제어 처리
- Raspberry Pi: 영상 처리 및 상위 제어 로직 수행
---
- 2차: Telechips 보드 기반 포팅 및 최적화

---

#  1차 프로젝트 — STM32 + Raspberry Pi 기반 구현

## 1. 유스케이스 시나리오
<img width="985" height="388" alt="Image" src="https://github.com/user-attachments/assets/2528a05a-772f-4463-94e9-4839cc617c3a" />

---

## 2. 요구사항 정의
<img width="1090" height="465" src="https://github.com/user-attachments/assets/593c3bc7-1fc3-4fc5-8737-0d059109f232" />

---

## 3. 시스템 아키텍처 (직접 개발 영역 표시)
<img width="1322" height="741" alt="Image" src="https://github.com/user-attachments/assets/4bf97ada-7169-4e8f-9cd1-723bd596cc27" />

- STM32 기반 FreeRTOS를 이용한 중앙 ECU 제어 구조 설계
- ADAS 기능(SCC, LFA, MCB)의 실시간 제어 로직 구현
- 센서 데이터 처리 및 차량 제어 흐름 구성
- CAN 통신 기반 차량 네트워크 일부 구현

---

## 4. 데모 환경
<img width="1026" height="465" src="https://github.com/user-attachments/assets/7f6f1c3c-b405-4f92-8b03-c440828151b2" />

---

## 5. 세부 개발 내용

### 5-1. RTOS 기반 중앙 제어 시스템
STM32와 FreeRTOS를 기반으로 차량의 중앙 제어 구조를 설계 및 구현
SCC, LFA, MCB 기능을 Task 단위로 분리하여 실시간 제어가 가능하도록 구성

- RTOS 기반 태스크 구조 설계 및 우선순위 구성
- 센서 데이터 수집 및 처리 구조 설계
- 차량 상태에 따른 제어 로직 흐름 구성

---

### 5-2. ADAS 제어 로직 구현 (SCC / LFA / MCB)

#### SCC (차간거리 제어)
- 전방 거리 기반 속도 제어 로직 구현
- 안전 거리 유지 로직 설계 ( timegap 사용 )

#### LKA (차선 유지 보조)
- 차선 편차 기반 조향 제어 로직 구현
- 차량 중심 유지 알고리즘 설계

#### MCB (충돌 방지 제동)
- 충돌 상황 판단 기반 제동 로직 구현
- 시스템 상태에 따른 긴급 제어 구조 설계

---

### 5-3. 차량 제어 및 통신 구조
- 속도 / 조향 / 제동 제어 로직 직접 구현
- 센서 입력 → 제어 판단 → 출력으로 이어지는 구조 설계
- CAN 통신을 일부 적용하여 차량 메시지 송수신 구조 구성

---

### 5-4. 압력 센서 기반 페달 입력 모듈
- SPI(spidev)를 이용해 압력 센서 값을 실시간 수집
- 센서 값을 가속/브레이크 입력 신호로 변환
- 일정 주기 기반으로 데이터 전송 및 임계값 필터링 적용

---

## 6. 시스템 동작 플로우차트
<img width="1277" height="716" alt="Image" src="https://github.com/user-attachments/assets/ca1c8061-4504-4086-bf86-4bcf0bf5e2d0" />

---

## 7. 실행 결과

### 7-1. Smart Cruise Control (SCC)

| 결과 1 | 결과 2 |
|--------|--------|
| <img src="https://github.com/user-attachments/assets/0c1eb2ba-20ca-4aed-b1c8-36ba61bba0b0" width="450"> | <img src="https://github.com/user-attachments/assets/80a570c7-febe-4be7-bb06-ec1d260eb8d7" width="450"> |
| 전방 거리 기반 속도 제어 동작 확인 | 안전거리에서의 제동 확인 |

---

### 7-2. Lane Following Assist (LFA)

| 결과 |
|------|
| <img src="https://github.com/user-attachments/assets/dcede31f-3710-467c-82ef-98b89146bb49" width="450"> |
| 차선 편차 기반 조향 제어 동작 확인 |

---

### 7-3. Multi-Collision Brake (MCB)

| 결과 1 | 결과 2 |
|--------|--------|
| <img src="https://github.com/user-attachments/assets/ce83d6e5-bbb0-4f07-abbc-61d5d16cb920" width="450"> | <img src="https://github.com/user-attachments/assets/5429fd8a-4736-4561-99bd-6bb3c1767956" width="450"> |
| 충돌 상황 판단 및 제동 제어 동작 확인 | 긴급 제동 로직 동작 확인 |


#  2차 프로젝트 — Telechips  보드 포팅 및 최적화

1차 STM32 기반 시스템을 Telechips 차량용 플랫폼으로 포팅하고  
FreeRTOS 기반 구조로 재구성하여 실시간 성능 개선

---

## 1. 시스템 아키텍처 (직접 개발 영역 표시)
<img width="1475" height="830" alt="Image" src="https://github.com/user-attachments/assets/79bcac5f-ff45-4773-8818-c4f9347a073f" />

- W5500 기반 Ethernet 통신 구조 설계 및 적용
- VCP-G 보드에서 RTOS 기반 시스템 최적화
- 센서 데이터 및 제어 신호의 실시간 전송 구조 구현

---

## 2. 세부 개발 내용

### 2-1. W5500 기반 이더넷 통신 시스템
- W5500 Ethernet 모듈을 활용한 TCP 통신 구조 구현
- VCP-G 보드에서 센서 데이터 및 제어 신호 송수신 처리
- 실시간 데이터 전송 구조 적용

---

### 2-2. RTOS 기반 시스템 구조 최적화
- FreeRTOS 기반 태스크 구조로 시스템 재구성
- 센서, 통신, 제어 로직을 Task 단위로 분리
- 실시간 처리 성능 및 시스템 안정성 개선

---

### 2-3. 날씨 API 기반 안전 알림 기능
- Wi-Fi 모듈을 이용한 네트워크 통신 환경 구성
- Weather API를 통해 실시간 기상 정보 수신
- 기상 데이터 기반 안전 운행 알림 로직 구현

---

### 2-4. UFLD 기반 차선 인식 최적화 (협업)
- UFLD 딥러닝 모델 기반 차선 인식 시스템에서 전/후처리 최적화 작업 수행
- 실시간 처리를 위한 입력 데이터 전처리 경량화
- 추론 결과 후처리 로직 개선을 통해 성능 안정화
- 팀원과 협업하여 전체 파이프라인 성능 개선

---

## 3. 실행 결과

### 3-1. SCC 안전거리 날씨 API 사용
| 결과 1 | 결과 2 |
|--------|--------|
| <img src="https://github.com/user-attachments/assets/4361772b-9e89-4fb7-9365-3df7273bfa61" width="450"> | <img src="https://github.com/user-attachments/assets/047315ca-14ed-41dc-9e36-bed9168e5f4e" width="450"> |
| 날씨에 따른 안전거리 단계 설정 경고 | 전방 거리 기반 속도 제어 동작 확인 |

---

### 3-2. UFLD 딥러닝 기반 차선 인식
| 결과 1 | 결과 2 |
|--------|--------|
| <img src="https://github.com/user-attachments/assets/ccafcd57-7cd1-48c0-ad2c-39bd15068a08" width="450"> | <img src="https://github.com/user-attachments/assets/36c644e2-2b9c-4edd-a857-cd636ed4dbe7" width="450"> |

- 1차 프로젝트에서의 opencv보다 FPS 4.6배 향상 및 정확도 개선
- 전후처리 최적화로 FPS 30 -> 40으로 증가 및 전체 시스템 지연 시간 25% 개선

---

## 전체 시스템 하드웨어 구성 (보드 및 센서 연결)
<img width="480" height="720" alt="Image" src="https://github.com/user-attachments/assets/3395d101-bbd9-4d99-bf47-0cf46c39d282" />

---

## 추가적인 내용 (CAN 통신 환경 구성)

- Raspberry Pi와 MCP2515 CAN 모듈을 사용하여 Linux SocketCAN 기반 CAN 통신 환경을 구성
```
cd /boot/firmware
sudo gedit config.txt

dtparam=spi=on
dtoverlay=mcp2515-can0,oscillator=12000000,interrupt=25,spimaxfrequency=2000000

sudo reboot

# CAN 드라이버 로드
sudo ip link set can0 up type can bitrate 500000

# 상태 확인
ifconfig can0

sudo apt install can-utils

ip -details link show can0

# CAN 메시지 송신 테스트
cansend can0 123#DEADBEEF

# CAN 메시지 수신 테스트
candump can0
```
<img width="602" height="370" alt="image" src="https://github.com/user-attachments/assets/6215498b-f468-4053-91b0-0f870f31424f" />

---

## 추가적인 내용 (임베디드 시스템 확장 및 통신 모듈 구현)

- VCP-G 보드 RTOS 환경에서 Task Suspend/Resume 기능 부재로 인해 직접 기능을 추가 및 확장 구현

  관련 정리: Notion 문서
  https://app.notion.com/p/TOPST-VCP-G-SAL-256697796dab81228e7fd7e2e3d155fd?source=copy_link

- W5500 기반 TCP 통신 구조 설계 및 SPI 기반 데이터 송수신 구현

  관련 정리: Notion 문서
  https://app.notion.com/p/TOPST-VCP-G-W5500-256697796dab81acbae6fe6c3aa2043b?source=copy_link

- Wi-Fi 모듈

  관련 정리: Notion 문서
  https://app.notion.com/p/TOPST-D3-wifi-256697796dab811cb3dbdedd1ecfcebd?source=copy_link

- 날씨 API 기반 실시간 기상 정보 수신

  관련 정리: Notion 문서
  https://app.notion.com/p/TOPST-D3-P-API-256697796dab813f9152c844c502f1c2?source=copy_link


- VCP-G 보드 RTOS 환경에서 Task Suspend/Resume 기능 부재로 인해 직접 기능을 추가 및 확장 구현  
    [Notion 문서](https://app.notion.com/p/TOPST-VCP-G-SAL-256697796dab81228e7fd7e2e3d155fd?source=copy_link)

- W5500 기반 TCP 통신 구조 설계 및 SPI 기반 데이터 송수신 구현  
    [Notion 문서](https://app.notion.com/p/TOPST-VCP-G-W5500-256697796dab81acbae6fe6c3aa2043b?source=copy_link)

- Wi-Fi 모듈  
    [Notion 문서](https://app.notion.com/p/TOPST-D3-wifi-256697796dab811cb3dbdedd1ecfcebd?source=copy_link)

- 날씨 API 기반 실시간 기상 정보 수신  
    [Notion 문서](https://app.notion.com/p/TOPST-D3-P-API-256697796dab813f9152c844c502f1c2?source=copy_link)

#  프로젝트 성과

- 임베디드 시스템 설계 및 RTOS 기반 다중 Task 구조 구현 경험 확보
- W5500 기반 Ethernet 통신 및 실시간 데이터 송수신 구조 설계
- CAN 통신 및 차량 네트워크 데이터 처리 경험 확보
- 영상 기반 차선 인식 및 전처리/후처리 최적화 (협업)
- Wi-Fi 모듈을 활용한 외부 Weather API 연동 및 데이터 기반 기능 구현
- 시스템 포팅 및 성능 최적화 경험 (STM32 → Telechips Board)
- 센서 및 GPIO 기반 디바이스 드라이버 구현 경험

---

#  기술 스택

- C/Python
- RTOS(freeRTOS)
- Embedded System
- Linux
- 딥러닝 기반 차선 인식
- TCP/IP 통신
- SPI 통신
- Can 통신
