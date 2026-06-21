pushd %~dp0
cd ..
where cl >nul 2>nul || (echo Run this from a Visual C++ Developer Command Prompt. & popd & exit /b 1)
where nmake >nul 2>nul || (echo nmake was not found in PATH. & popd & exit /b 1)
set tgt=nmake
if /I "%1"=="xp"    set tgt=nmake-xp
if /I "%1"=="win64" set tgt=nmake-win64
cmake --preset %tgt%
cmake --build --preset %tgt%-install
if /I "%tgt%"=="nmake-win64" cmake --build --preset %tgt%-package
popd
