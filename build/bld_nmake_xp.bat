pushd %~dp0
cd ..
where cl >nul 2>nul || (echo Run this from a v141_xp-capable Visual C++ Developer Command Prompt. & popd & exit /b 1)
where nmake >nul 2>nul || (echo nmake was not found in PATH. & popd & exit /b 1)
set tgt=vc-xp
cmake --preset %tgt%
cmake --build --preset %tgt%-install
cmake --build --preset %tgt%-package
popd
