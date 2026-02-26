#!/bin/bash

# 1. 사용자 입력 (디렉토리명 설정)
SRC_DIR="25_stm8s103_debug"
DST_DIR="27_stm8s105_debug"

# 2. 프로젝트 이름 자동 추출 (앞의 '숫자_' 제거)
OLD_NAME="${SRC_DIR#*_}"
NEW_NAME="${DST_DIR#*_}"

echo "===================================================="
echo " [Project Clone & Rename]"
echo " - Source: $SRC_DIR ($OLD_NAME)"
echo " - Target: $DST_DIR ($NEW_NAME)"
echo "===================================================="

# 3. 대상 디렉토리 및 src 폴더 생성
mkdir -p "$DST_DIR/src"

# 4. 소스 디렉토리 존재 여부 선제 확인
if [ ! -d "$SRC_DIR" ]; then
    echo "❌ Error: Source directory $SRC_DIR not found."
    exit 1
fi

# 5. 프로젝트 설정 파일 처리 (makefile)
echo "[CONFIG] Processing Configuration Files..."
SRC_MAKEFILE="$SRC_DIR/makefile"
DST_MAKEFILE="$DST_DIR/makefile"

if [ -f "$SRC_MAKEFILE" ]; then
    echo "[FILE] Processing makefile: $OLD_NAME -> $NEW_NAME"
    sed "s|$OLD_NAME|$NEW_NAME|g" "$SRC_MAKEFILE" > "$DST_MAKEFILE"
fi

# 6. 소스 파일 및 메인 파일 처리 (src 폴더)
echo "[SRC ] Processing Source Files..."

if [ -d "$SRC_DIR/src" ]; then
    # src 폴더 내의 모든 파일을 순회하며 복사 및 이름 변경
    for FILE_PATH in "$SRC_DIR/src"/*; do
        FILENAME=$(basename "$FILE_PATH")
        
        # 파일명이 OLD_NAME_main.c 인 경우 NEW_NAME_main.c 로 변경
        if [ "$FILENAME" == "${OLD_NAME}_main.c" ]; then
            TARGET_FILENAME="${NEW_NAME}_main.c"
            echo "  -> Renaming Main: $FILENAME -> $TARGET_FILENAME"
            # 파일 내용 내의 프로젝트 이름도 함께 치환하여 저장
            sed "s|$OLD_NAME|$NEW_NAME|g" "$FILE_PATH" > "$DST_DIR/src/$TARGET_FILENAME"
        else
            # 그 외의 파일(interrupt_vector.c 등)은 그대로 복사 (필요시 내부 텍스트만 치환)
            echo "  -> Copying: $FILENAME"
            # 내부 텍스트에 프로젝트 이름이 포함되어 있을 수 있으므로 sed 처리 권장
            sed "s|$OLD_NAME|$NEW_NAME|g" "$FILE_PATH" > "$DST_DIR/src/$FILENAME"
        fi
    done
else
    echo "⚠️  Warning: $SRC_DIR/src 디렉토리를 찾을 수 없습니다."
fi

echo "===================================================="
echo " ✅ 작업 완료! ./$DST_DIR 폴더를 확인해 보세요."
echo "===================================================="