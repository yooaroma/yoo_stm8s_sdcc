#!/bin/bash

# 1. obj 디렉토리 내에서 첫 번째 .map 파일을 자동으로 찾습니다.
MAP_FILE=$(ls obj/*.map 2>/dev/null | head -n 1)

# 파일이 없을 경우 에러 메시지 출력
if [ -z "$MAP_FILE" ]; then
    echo "[ERROR] obj 디렉토리에서 .map 파일을 찾을 수 없습니다."
    exit 1
fi

echo "=================================================="
echo "  STM8S103 Build Size Summary (Auto-detect)"
echo "  Found File: $MAP_FILE"
echo "=================================================="

# 2. awk를 사용하여 크기 계산 및 % 산출
awk '
BEGIN {
    # STM8S103F3 하드웨어 제한 (Bytes)
    MAX_FLASH = 8192;
    MAX_RAM = 1024;
    MAX_EEPROM = 640;

    # 계산할 섹션 정의 (사용하시는 컴파일러 특성에 맞춰 정의됨)
    flash_segs["HOME"]=1; flash_segs["GSINIT"]=1; flash_segs["GSFINAL"]=1;
    flash_segs["CONST"]=1; flash_segs["INITIALIZER"]=1; flash_segs["CODE"]=1;
    
    ram_segs["DATA"]=1; ram_segs["INITIALIZED"]=1; ram_segs["SSEG"]=1;
    
    eeprom_segs["EEPROM"]=1; eeprom_segs["ABS_EEPROM"]=1;

    flash_total = 0;
    ram_total = 0;
    eeprom_total = 0;
}

# 정규식 매칭: "이름  주소  크기 = 숫자. bytes" 형태의 라인 추출
/^[A-Z._]+ +[0-9A-F]+ +[0-9A-F]+ = +[0-9]+\. bytes/ {
    name = $1;
    size = $5;
    sub(/\./, "", size); # 숫자 뒤의 마침표 제거

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
    # 퍼센트 계산
    flash_pct = (flash_total / MAX_FLASH) * 100;
    ram_pct = (ram_total / MAX_RAM) * 100;
    eeprom_pct = (MAX_EEPROM > 0) ? (eeprom_total / MAX_EEPROM) * 100 : 0;

    print "--------------------------------------------------";
    printf "TOTAL FLASH  : %5d / %d Bytes (%5.1f%%)\n", flash_total, MAX_FLASH, flash_pct;
    printf "TOTAL RAM    : %5d / %d Bytes (%5.1f%%)\n", ram_total, MAX_RAM, ram_pct;
    printf "TOTAL EEPROM : %5d / %d Bytes (%5.1f%%)\n", eeprom_total, MAX_EEPROM, eeprom_pct;
    
    # 경고 출력
    if (flash_total > MAX_FLASH)  print "!!! [WARNING] FLASH OVERFLOW !!!";
    if (ram_total > MAX_RAM)      print "!!! [WARNING] RAM OVERFLOW !!!";
    if (eeprom_total > MAX_EEPROM) print "!!! [WARNING] EEPROM OVERFLOW !!!";
}
' "$MAP_FILE"

echo "=================================================="