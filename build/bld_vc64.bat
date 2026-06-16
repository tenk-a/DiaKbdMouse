pushd %~dp0
cd ..
set tgt=vc-x64
cmake --preset %tgt%
cmake --build --preset %tgt%-install
cmake --build --preset %tgt%-package
popd
