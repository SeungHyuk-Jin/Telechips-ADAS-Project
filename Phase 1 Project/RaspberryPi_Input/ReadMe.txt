# Raspberry Pi 버튼 및 디스플레이 실행 가이드

이 문서는 라즈베리파이에서 버튼 및 압력센서 기능을 설정하고 실행하기 위한 절차를 설명합니다.

---

## 1. 디바이스 트리 오버레이 설정

`buttons_overlay.dts` 파일을 컴파일하여 `.dtbo` 파일을 생성하고, 시스템 부팅 시 적용되도록 설정합니다.

# 디바이스 트리 컴파일
dtc -@ -I dts -O dtb -o buttons_overlay.dtbo buttons_overlay.dts

# 오버레이 디렉토리에 복사
sudo cp buttons_overlay.dtbo /boot/overlays/

# config.txt에 오버레이 적용 설정 추가
echo "dtoverlay=buttons_overlay" | sudo tee -a /boot/firmware/config.txt

# 시스템 재부팅
sudo reboot

------------------------------------------------------------------------------------------------------------------------------
## 2. 커널 모듈 빌드 및 로드
buttons_driver.c와 Makefile을 사용해 커널 모듈을 빌드하고 시스템에 로드합니다.

# 커널 모듈 빌드
make

# 모듈 삽입
sudo insmod buttons_driver.ko

# 디바이스 파일 생성 (major 번호는 환경에 따라 다를 수 있음, 여기선 236 기준)
sudo mknod /dev/buttons_dev c 236 0

------------------------------------------------------------------------------------------------------------------------------
##3. 애플리케이션 컴파일
gcc -o last_last last_last.c

------------------------------------------------------------------------------------------------------------------------------
##4. 실행
./last_last