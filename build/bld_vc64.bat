pushd %~dp0
cd ..
set tgt=vc%1-x64
cmake --preset %tgt%
cmake --build --preset %tgt%-install
if "%1"=="" cmake --build --preset %tgt%-package
popd
