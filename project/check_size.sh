#!/bin/bash

# 1. 사용자 입력 확인 (기본값 103)
CHIP_TYPE=${1:-"STM8S103"}  # 인자가 없으면 기본값으로 103 사용

# 2. 칩셋에 따른 하드웨어 사양 설정
if [ "$CHIP_TYPE" == "STM8S103" ]; then
    MAX_FLASH=8192
    MAX_RAM=1024
    MAX_EEPROM=640
    CHIP_NAME="STM8S103F3"
elif [ "$CHIP_TYPE" == "STM8S105" ]; then
    MAX_FLASH=16384
    MAX_RAM=2048
    MAX_EEPROM=1024
    CHIP_NAME="STM8S105K4"
elif [ "$CHIP_TYPE" == "STM8S105C6" ]; then
    MAX_FLASH=32768
    MAX_RAM=2048
    MAX_EEPROM=1024
    CHIP_NAME="STM8S105C6"
else
    echo "[ERROR] 지원하지 않는 칩셋입니다: $CHIP_TYPE (STM8S103 또는 STM8S105 또는 STM8S105C6 입력)"
    exit 1
fi

# 3. obj 디렉토리 내에서 .map 파일 자동 찾기
MAP_FILE=$(ls obj/*.map 2>/dev/null | head -n 1)

if [ -z "$MAP_FILE" ]; then
    echo "[ERROR] obj 디렉토리에서 .map 파일을 찾을 수 없습니다."
    exit 1
fi

echo "=================================================="
echo "  $CHIP_NAME Build Size Summary"
echo "  Found File: $MAP_FILE"
echo "=================================================="

# 4. awk를 사용하여 크기 계산 (Bash 변수를 -v 옵션으로 전달)
awk -v M_FLASH="$MAX_FLASH" -v M_RAM="$MAX_RAM" -v M_EEPROM="$MAX_EEPROM" '
BEGIN {
    MAX_FLASH = M_FLASH;
    MAX_RAM = M_RAM;
    MAX_EEPROM = M_EEPROM;

    # 세그먼트 정의
    flash_segs["HOME"]=1; flash_segs["GSINIT"]=1; flash_segs["GSFINAL"]=1;
    flash_segs["CONST"]=1; flash_segs["INITIALIZER"]=1; flash_segs["CODE"]=1;
    ram_segs["DATA"]=1; ram_segs["INITIALIZED"]=1; ram_segs["SSEG"]=1;
    eeprom_segs["EEPROM"]=1; eeprom_segs["ABS_EEPROM"]=1;

    flash_total = 0; ram_total = 0; eeprom_total = 0;
}

/^[A-Z._]+ +[0-9A-F]+ +[0-9A-F]+ = +[0-9]+\. bytes/ {
    name = $1;
    size = $5;
    sub(/\./, "", size);

    if (!(name in seen)) {
        seen[name] = 1;
        if (name in flash_segs) {
            flash_total += size;
            printf "%-12s : %5d bytes\n", name, size;
        }
        else if (name in ram_segs) {
            ram_total += size;
            printf "%-12s : %5d bytes (RAM)\n", name, size;
        }
        else if (name in eeprom_segs) {
            eeprom_total += size;
            printf "%-12s : %5d bytes (EEPROM)\n", name, size;
        }
    }
}

END {
    flash_pct = (flash_total / MAX_FLASH) * 100;
    ram_pct = (ram_total / MAX_RAM) * 100;
    eeprom_pct = (MAX_EEPROM > 0) ? (eeprom_total / MAX_EEPROM) * 100 : 0;

    print "--------------------------------------------------";
    printf "TOTAL FLASH  : %5d / %d Bytes (%5.1f%%)\n", flash_total, MAX_FLASH, flash_pct;
    printf "TOTAL RAM    : %5d / %d Bytes (%5.1f%%)\n", ram_total, MAX_RAM, ram_pct;
    printf "TOTAL EEPROM : %5d / %d Bytes (%5.1f%%)\n", eeprom_total, MAX_EEPROM, eeprom_pct;
    
    if (flash_total > MAX_FLASH)  print "!!! [WARNING] FLASH OVERFLOW !!!";
    if (ram_total > MAX_RAM)      print "!!! [WARNING] RAM OVERFLOW !!!";
    if (eeprom_total > MAX_EEPROM) print "!!! [WARNING] EEPROM OVERFLOW !!!";
}
' "$MAP_FILE"

echo "=================================================="