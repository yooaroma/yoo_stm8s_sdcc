# ST Microcontroller Development Guide
**VS Code + SDCC + Make를 활용한 STM8S 시리즈 개발 환경**

---

## 1. 필수 설치 도구 (Tools)

STM8 개발을 위해 다음 도구들을 설치하고 환경 변수를 설정해야 합니다.

* **Editor:** [VS Code 다운로드](https://code.visualstudio.com/download)
* **Compiler (ST):** [SDCC (Snapshot 빌드 권장)](https://sourceforge.net/projects/sdcc/files/snapshot_builds/x86_64-w64-mingw32/)
    * `C:\tools\sdcc\bin` 환경변수 Path 추가 필수
* **Build Tool:** [Make (Windows Build Tools)](https://github.com/xpack-dev-tools/windows-build-tools-xpack/releases)
    * `C:\tools\make\bin` 환경변수 Path 추가 필수
* **Compiler (GCC):** [MinGW-w64 다운로드](https://github.com/niXman/mingw-builds-binaries/releases)
* **Programmer:** [ST Visual Programmer (STVP)](https://www.st.com/en/development-tools/stvp-stm8.html)
* **Flasher:** [stm8flash 직접 다운로드](https://yooaroma.com/tools/stm8flash.zip)
* **VS Code Extension:** Serial Monitor, STM8-debug

---

## 2. 레퍼런스 및 소스코드

### 📌 공식 소스코드 저장소
* [GitHub - yooaroma/yoo_stm8s_sdcc](https://github.com/yooaroma/yoo_stm8s_sdcc)

### 📚 데이터시트 및 매뉴얼
| 문서명 | 한국어/기타 | 영어 (ENG) |
| :--- | :---: | :---: |
| **STM8S103F3 Chip Manual** | [Link](https://yooaroma.com/mzip/stm8s_chip.pdf) | [Link](https://www.st.com/resource/en/datasheet/stm8s103f3.pdf) |
| **Programming Manual (PM0044)** | [Link](https://yooaroma.com/mzip/stm8s_pgm.pdf) | [Link](https://www.st.com/resource/en/programming_manual/pm0044-stm8-cpu-programming-manual-stmicroelectronics.pdf) |
| **Reference Manual** | [Link](https://yooaroma.com/mzip/stm8s_ref.pdf) | [Link](https://www.st.com/resource/en/reference_manual/rm0016-stm8s-series-and-stm8af-series-8bit-microcontrollers-stmicroelectronics.pdf) |
| **STM8S C example** | [Link](https://yooaroma.com/mzip/stm8s_yoo.pdf) | - |

---

## 3. 하드웨어 주요 사양 (STM8S103F3P6)

[Image of STM8S103F3P6 development board pinout]

* **Board Front:** [이미지 확인](https://yooaroma.com/mm/image/stm8/stm8blue/STM8S103F3P6_BOARD.png)
* **Pinout Diagram:** [이미지 확인](https://yooaroma.com/mm/image/stm8/stm8blue/STM8S-Blue-Generic-STM8S103F3P6-Microcontroller-Board-Pinout-Diagram-R0.1-CIRCUITSTATE-Electronics-1.png)
* **회로도:** [이미지 확인](https://yooaroma.com/mzip/01_stm8s103_void_sch.png)

---

## 4. 하드웨어 핀 맵 (Pin Mapping)

### ⬅️ LEFT Header
* **PD4**: D13
* **PD5**: D14 / A3 / **UART_TXD**
* **PD6**: D15 / A4 / **UART_RXD**
* **RST**: NRST (Reset)
* **PA1**: D0 / KEY1
* **PA2**: D1 / LED1
* **GND**: Ground
* **5V**: Power In
* **3.3V**: Power Out
* **PA3**: D2 / SPI_NSS

### RIGHT Header ➡️
* **PD3**: D23 / A2 / **KEY**
* **PD2**: D11 / A1
* **PD1**: D10 / **SWIM**
* **PC7**: D9 / SPI_MISO
* **PC6**: D8 / SPI_MOSI
* **PC5**: D7 / SPI_SCK
* **PC4**: D6 / A0
* **PC3**: D5
* **PB4**: D4 / **I2C_SCL**
* **PB5**: **LED** / **I2C_SDA**

---

## 5. 실행 순서 (Workflow)

### 🛠 빌드 및 업로드
1.  터미널에서 프로젝트 디렉토리로 이동합니다.
2.  `make`를 입력하여 컴파일 및 `bin/xx.hex`를 생성합니다.
3.  `make flash`를 입력하여 업로드를 수행합니다 (stm8flash 도구 사용).

### 🌿 Git 작업 순서
```bash
git add .
git commit -m "작업 내용 기록"
git push
```
<br>
© 2024 STM8 Development Guide | Reference: yooaroma.com <br>
