#!/bin/bash

# 1. Debug 디렉토리 내에서 첫 번째 .map 파일을 자동으로 찾습니다.
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

# 2. awk를 사용하여 중복 방지 및 크기 계산 실행
# $1: 섹션이름, $2: 시작주소, $3: 크기(Hex), $5: 크기(Dec)
awk '
BEGIN {
    # 계산할 섹션 정의
    flash_segs["HOME"]=1; flash_segs["GSINIT"]=1; flash_segs["GSFINAL"]=1;
    flash_segs["CONST"]=1; flash_segs["INITIALIZER"]=1; flash_segs["CODE"]=1;
    ram_segs["DATA"]=1; ram_segs["INITIALIZED"]=1; ram_segs["SSEG"]=1;
    
    flash_total = 0;
    ram_total = 0;
}

# 정규식 매칭: "이름  주소  크기 = 숫자. bytes" 형태의 라인만 추출
/^[A-Z._]+ +[0-9A-F]+ +[0-9A-F]+ = +[0-9]+\. bytes/ {
    name = $1;
    size = $5;
    sub(/\./, "", size); # 숫자 뒤의 마침표 제거

    # 이미 처리한 섹션이 아닐 경우에만 합산 (첫 번째 요약 테이블만 사용)
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
    }
}

END {
    print "--------------------------------------------------";
    printf "TOTAL FLASH : %5d Bytes (%.2f KB)\n", flash_total, flash_total/1024;
    printf "TOTAL RAM   : %5d Bytes\n", ram_total;
    
    if (flash_total > 8192) {
        print "!!! FLASH OVERFLOW (Limit: 8KB) !!!";
    }
}
' "$MAP_FILE"

echo "=================================================="