pushd %~dp0
cd ..
set tgt=watcom
cmake --preset %tgt%
cmake --build --preset %tgt%-install
cmake --build --preset %tgt%-package
popd
