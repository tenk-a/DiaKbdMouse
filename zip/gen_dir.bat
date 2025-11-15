rem echo off
setlocal
pushd %~dp0

mkdir DiaKbdMouse%1
pushd DiaKbdMouse%1

copy /b ..\..\DiaKbdMouse.cfg .\
copy /b ..\..\DiaKbdMouse.htm .\
copy /b ..\..\install.bat .\
copy /b ..\..\regist_startup.bat .\
copy /b ..\..\README.md .\
xcopy ..\..\config_etc .\config_etc /R /Y /I /K /E
xcopy ..\..\inst .\inst /R /Y /I /K /E

popd

popd
endlocal
