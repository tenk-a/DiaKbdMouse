@echo off
@setlocal
@pushd %~dp0

:INP_START
@echo -------------------------------------------
@echo DiaKbdMouse のインストール(exe,dllのコピー)
@echo -------------------------------------------
@echo 1. Windows Vista/7/8/8.1/10 x64 用に設定.
@echo 2. Windows Vista/7/8/8.1/10 x86 用に設定.
@echo 3. Windows 2k/xp x86 用に設定.
@echo -
@set INP=
@set /P INP="1,2,3 いずれかの番号を入力してください。"
@if "%INP%"=="1" goto WIN64
@if "%INP%"=="2" goto WIN32
@if "%INP%"=="3" goto WIN32_w2kXp
@goto INP_START

:WIN64
@copy /b inst\win64\*.* .\
@goto :END

:WIN32
@copy /b inst\win32\*.* .\
@goto :END

:WIN32_w2kXp
@copy /b inst\win32_w2kXp\*.* .\
@goto :END

:END
@pause

@popd
@endlocal
