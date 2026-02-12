# ST 마이컴 개발시 설치 TOOLS

- vscode download : https://code.visualstudio.com/download
- 칩(chip) 프로그래머 : ST Visual Programmer : <https://www.st.com/en/development-tools/stvp-stm8.html>
- st 컴파일러 : sdcc download : <https://sourceforge.net/projects/sdcc/files/snapshot_builds/x86_64-w64-mingw32/>
  - snapshot 의 빌더 버젼(\*.zip)을 다운로드 받아서 unzip하고 나서 c:/tools 아래에 카피한다. 그리고 시스템 환경변수 path 에 위 c:\tools\sdcc\bin 폴더를 추가한다.
- make : download : <https://github.com/xpack-dev-tools/windows-build-tools-xpack/releases>
  - 최신 버젼을 (\*.zip) 화일을 unzip 하여 c:/tools/make 아래에 카피한다. 그리고 시스템 환경변수 path 에 위 c:\tools\make\bin 폴더를 추가한다.
- gcc 컴파일러 설치 : <https://www.mingw-w64.org/>
  - 다운로드로 가서 : <https://www.mingw-w64.org/downloads/> -> <https://github.com/niXman/mingw-builds-binaries/releases> 즉 Mingw-builds installation: GitHub 를 선택해서 path는 c:\tools\mingw64 에 설치하고 사용하기를 권장...
  - path c:\tools\mingw64\bin 를 추가하면 실행이 가능합니다.
- vscode extention program : Serial Monitor

---

# STM8S_DISCOVERY

- site : <https://www.st.com/en/evaluation-tools/stm8s-discovery.html>
- vscode key : <https://keycombiner.com/collections/vscode/>

---

![STM8S-DISCOVERY](https://www.st.com/bin/ecommerce/api/image.PF247087.en.feature-description-include-personalized-no-cpn-large.jpg)

---

# All features

- <https://www.st.com/content/ccc/fragment/product_related/rpn_information/product_circuit_diagram/group3/95/7f/7f/bb/d4/44/4d/e4/bd-stm8s105x6-32k/files/bd-stm8s105x6-32k.jpg/jcr:content/translations/en.bd-stm8s105x6-32k.jpg>
- STM8S105C6T6 microcontroller, 32 KB Flash, 2 KB RAM, 1 KB EEPROM
- Powered by USB cable between PC and STM8S-DISCOVERY
- Selectable power of 5 V or 3.3 V
- Touch sensing button
- User LED
- Extension header for all I/Os
- Wrapping area for users own application
- Embedded ST-Link
- USB interface for programming and debugging
- SWIM debug support

---

# 참고 문서

- STM8S-DISCOVERY User Manual : <https://www.st.com/resource/en/user_manual/um0817-stm8sdiscovery-stmicroelectronics.pdf>
- Developing and debugging : <https://www.st.com/resource/en/user_manual/um0834-developing-and-debugging-your-stm8sdiscovery-application-code-stmicroelectronics.pdf>
- debugging and writer : <https://www.devicemart.co.kr/goods/view?no=11871401>
- 부품 사양
- STM8S105C6T6 : <https://www.st.com/en/microcontrollers-microprocessors/stm8s105c6.html>
  - STM8S Reference Manuals : <https://www.st.com/resource/en/reference_manual/rm0016-stm8s-series-and-stm8af-series-8bit-microcontrollers-stmicroelectronics.pdf>
  - pdf <https://www.st.com/resource/en/datasheet/stm8s105c6.pdf>
  - doc link : <https://www.st.com/en/microcontrollers-microprocessors/stm8s105c6.html#documentation>
  - Programming Manuals : <https://www.st.com/resource/en/programming_manual/pm0044-stm8-cpu-programming-manual-stmicroelectronics.pdf>
  - PB5 : PORT : VCC - PU - VOUT - SW - GND / VOUT - LED - 100R - GND
- The STM8S Series and STM8AF Series Flash programming manual (PM0051)
  - <https://www.st.com/resource/en/programming_manual/pm0051-how-to-program-stm8s-and-stm8a-flash-program-memory-and-data-eeprom-stmicroelectronics.pdf>
- The STM8 SWIM communication protocol and debug module user manual (UM0470)
  - <https://www.st.com/resource/en/user_manual/cd00173911-stm8-swim-communication-protocol-and-debug-module-stmicroelectronics.pdf>
- STM8 bootloader user manual (UM0560)
  - <https://www.st.com/resource/en/user_manual/cd00201192-stm8-bootloader-stmicroelectronics.pdf>
- STM8 CPU programming manual (PM0044)
  - <https://www.st.com/resource/en/programming_manual/pm0044-stm8-cpu-programming-manual-stmicroelectronics.pdf>
- STM8 in-application programming example (AN2659)
  - <https://www.st.com/resource/en/application_note/an2659-stm8-inapplication-programming-iap-using-a-customized-userbootloader-stmicroelectronics.pdf>

---

# 기타 자료

- stm8flash : <https://github.com/hbendalibraham/stm8_started>
- 디버거 : <https://marketplace.visualstudio.com/items?itemName=CL.stm8-debug>

---

# 참고자료

- snippets : 사용법 <https://youtu.be/EVxCdenPbFs> 드림코딩
- devicemart 구매
- [SMG] Micro USB STM8S103F3P6 초소형 개발보드 [SZH-AT041] <https://www.devicemart.co.kr/goods/view?no=1361337>
- [SMG] STM8S105K4T6 초소형 개발보드 [SZH-DVBP-002] <https://www.devicemart.co.kr/goods/view?no=1326909>
- [SMG] ST-LINK V2 호환 STM8/STM32 프로그래머/에뮬레이터 [SZH-PRBP-004] <https://www.devicemart.co.kr/goods/view?no=1326910>
- [SMG] CH340G USB to TTL 컨버터 모듈 [SZH-EK092] <https://www.devicemart.co.kr/goods/view?no=1321026>

---

# 실행 순서

- 하고자 하는 디렉토리로 이동한다.
- make 하면 makefile을 참고하여 실행한다.
- 결과 화일은 obj 와 exe에 생성된다.
- xx.hex 화일을 다운로드한다.
- ST Visual Programmer 을 통해서 화일을 읽어 들인다.
- File -> Open (xx.hex)
- Configure 선택 (ST-LINK - USB - SWIM - STM8S105x6) OK
- Program -> Current Tab (Ctl-P)
- File -> Exit

---

# 단축 키

- 반복되는 단어 한방에 수정 Multi Selection : **Ctrl + D**
- 클릭하는 곳마다 커서 생성 Insert cursor : **Alt + Click**
- 코드 위/아래로 움직이기 Move line down / up : **Alt + up / down**
- 코드 복사해서 위/아래로 움직이기 Copy line down / up : **Alt + Shift + up / down**
- 코드 블록 한방에 코멘트 처리하기 : **Ctrl + /**
- 선택된 영역에 커서 만들기 Current selection : **Alt + Shift + i**
- 마우스가 가는 곳 마다 커서 만들기 : **Alt + Shift + Mouse Drag**
- 파일 맨 위-아래로 한번에 이동하기 Beginning / end of file : **Ctrl + Home / End**
- 라인 맨 처음 과 끝 beginnig / end of line : **Home / End**
- 단어 이동 Move Word : **Ctrl + left / right**
- 단어 선택 Select Word : **Ctrl + Shift + left / right**
- 사이드바 숨기기 Toggle Sidebar : **Ctrl + B**
- 터미널 가기 Toggle Terminal : **Ctrl + ~**
- 마크다운 Preview : **Ctrl+ Shift + v**
- 행 삭제 : **Ctrl + Shift + k**
- 행을 아랫줄로 이동 : **Alt + down**
- 행을 윗줄로 이동 : **Alt + up**
- 아래에 행 삽입 Insert line below : **Ctrl + enter**
- 위에 행 삽입 : **Ctrl + Shift + enter**
- 커서를 아래에 추가 : **Ctrl + Alt + down**
- 커서를 위에 추가 : **Ctrl + Alt + up**
- 현재 선택 항목을 모두 선택 : **Ctrl + Shift + l**
- 라인 들여 쓰기 : **Ctrl + ]**
- 라인 내어 쓰기 : **Ctrl + [**
- 커서 위치에 주석 토글 : **Alt + Shift + a**
- 코맨드 팔렛트 : **F1 or Ctrl + Shift + p**
- Quick Open : **Ctrl + p**
- User Settings : **Ctrl + ,**
- Keyboard Shortcuts : **Ctrl + k + s**
- Undo last Cursor : **Ctrl + u**
  단축키 정리 (윈도우용) ⇢ (https://code.visualstudio.com/shortcuts/keyboard-shortcuts-windows.pdf)

---

Use SDCC Toolchain :
Notice: make sure you can find your 'target' and 'interface' in OpenOcd config folder !, like: 'target/stm8s003.cfg'

{
"version": "0.2.0",
"configurations": [
{
"type": "stm8-debug",
"request": "launch",
"name": "Launch Program",
"serverType": "stm8-sdcc",
"executable": ".\\out\\Debug\\stm8_demo.elf",
"openOcdConfigs": [
"interface/stlink.cfg",
"target/stm8s003.cfg"
]
}
]
}
