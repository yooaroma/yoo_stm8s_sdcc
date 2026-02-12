@echo off
setlocal enabledelayedexpansion

:: 1. Debug 디렉토리 내에서 첫 번째 .map 파일을 자동으로 찾습니다.
set "MAP_FILE="
for %%F in (Debug\*.map) do (
    if not defined MAP_FILE set "MAP_FILE=%%F"
)

:: 파일이 없을 경우 에러 메시지 출력
if not defined MAP_FILE (
    echo [ERROR] Debug 디렉토리에서 .map 파일을 찾을 수 없습니다.
    pause
    exit /b
)

echo ==================================================
echo  STM8S103 Build Size Summary (Auto-detect)
echo  Found File: %MAP_FILE%
echo ==================================================

:: 2. PowerShell을 사용하여 중복 방지 로직 실행
powershell -Command ^
    "$flash_segs = @('HOME', 'GSINIT', 'GSFINAL', 'CONST', 'INITIALIZER', 'CODE');" ^
    "$ram_segs = @('DATA', 'INITIALIZED', 'SSEG');" ^
    "$seen = @{};" ^
    "$flash_total = 0;" ^
    "$ram_total = 0;" ^
    "Get-Content '%MAP_FILE%' | ForEach-Object {" ^
    "    if ($_ -match '^(\w+)\s+[0-9A-F]+\s+[0-9A-F]+\s+=\s+(\d+)\.') {" ^
    "        $name = $Matches[1];" ^
    "        $size = [int]$Matches[2];" ^
    "        if (-not $seen.ContainsKey($name)) {" ^
    "            $seen[$name] = $true;" ^
    "            if ($flash_segs -contains $name) {" ^
    "                $flash_total += $size;" ^
    "                Write-Host ('{0,-12} : {1,5} bytes' -f $name, $size);" ^
    "            }" ^
    "            elseif ($ram_segs -contains $name) {" ^
    "                $ram_total += $size;" ^
    "                Write-Host ('{0,-12} : {1,5} bytes (RAM)' -f $name, $size);" ^
    "            }" ^
    "        }" ^
    "    }" ^
    "};" ^
    "Write-Host '--------------------------------------------------';" ^
    "Write-Host ('TOTAL FLASH : {0,5} Bytes ({1:N2} KB)' -f $flash_total, ($flash_total/1024));" ^
    "Write-Host ('TOTAL RAM   : {0,5} Bytes' -f $ram_total);" ^
    "if ($flash_total -gt 8192) { Write-Host '!!! FLASH OVERFLOW (Limit: 8KB) !!!' -ForegroundColor Red }"

echo ==================================================
pause