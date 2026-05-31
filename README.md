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
- ADAS 기능(LKA, SCC, MCB)의 실시간 제어 로직 구현
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

### 6-1. Smart Cruise Control (SCC)

| 결과 1 | 결과 2 |
|--------|--------|
| <img src="https://github.com/user-attachments/assets/0c1eb2ba-20ca-4aed-b1c8-36ba61bba0b0" width="450"> | <img src="https://github.com/user-attachments/assets/80a570c7-febe-4be7-bb06-ec1d260eb8d7" width="450"> |
| 전방 거리 기반 속도 제어 동작 확인 | 안전거리에서의 제동 확인 |

---

### 6-2. Lane Following Assist (LFA)

| 결과 |
|------|
| <img src="https://github.com/user-attachments/assets/dcede31f-3710-467c-82ef-98b89146bb49" width="450"> |
| 차선 편차 기반 조향 제어 동작 확인 |

---

### 6-3. Multi-Collision Brake (MCB)

| 결과 1 | 결과 2 |
|--------|--------|
| <img src="https://github.com/user-attachments/assets/ce83d6e5-bbb0-4f07-abbc-61d5d16cb920" width="450"> | <img src="https://github.com/user-attachments/assets/5429fd8a-4736-4561-99bd-6bb3c1767956" width="450"> |
| 충돌 상황 판단 및 제동 제어 동작 확인 | 긴급 제동 로직 동작 확인 |
