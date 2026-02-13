#!/bin/bash

# 수정 대상 문자열
# 치환할 문자열 설정 (실제 값으로 변경하여 사용하세요)
OLD_STR='/project/'
NEW_STR='/'

# 특수 문자 이스케이프 함수 (sed 예약어 처리)
escape_sed() {
  echo "$1" | sed -e 's/[]\/$*.^[]/\\&/g'
}

ESCAPED_OLD=$(escape_sed "$OLD_STR")
ESCAPED_NEW=$(escape_sed "$NEW_STR")

# 모든 하위 디렉토리의 makefile 검색 및 수정
find . -type f -iname "makefile" | while read -r file; do
    if grep -qF "$OLD_STR" "$file"; then
        echo "수정 중: $file"
        # -i 옵션으로 내용 치환 (Linux GNU sed 기준)
        sed -i "s|$ESCAPED_OLD|$ESCAPED_NEW|g" "$file"
    fi
done

echo "✅ 모든 작업이 완료되었습니다."