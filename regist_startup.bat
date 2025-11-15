@echo off
chcp 65001 >nul
rem  DiaKbdMouse をログオン時に自動起動するタスクを登録.
setlocal

rem 管理者権限チェック.
net session >nul 2>&1
if %errorlevel% neq 0 (
    powershell -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    exit /b
)

set "TASK_NAME=DiaKbdMouse"
set "DELAY=0000:30"
set "DIA_EXE=%~dp0\DiaKbdMouse.exe"

if not exist "%DIA_EXE%" (
    echo [ERROR] %DIA_EXE% が見つかりません.
    goto :END
)

echo.
echo タスク "%TASK_NAME%" を作成します.
echo 実行ファイル: "%DIA_EXE%"
echo 遅延時間    : %DELAY%
echo.

schtasks /Delete /TN "%TASK_NAME%" /F >nul 2>&1

schtasks ^
 /Create ^
 /TN "%TASK_NAME%" ^
 /SC ONLOGON ^
 /TR "\"%DIA_EXE%\"" ^
 /RL HIGHEST ^
 /DELAY %DELAY% ^
 /IT ^
 /F

if errorlevel 1 (
    echo.
    echo [ERROR] タスク "%TASK_NAME%" の作成に失敗しました.
    goto :END
)

echo.
echo [OK] タスク "%TASK_NAME%" を作成しました.
echo 次回ログオン時から、DiaKbdMouse が自動起動します.
echo.

:END
pause
endlocal
