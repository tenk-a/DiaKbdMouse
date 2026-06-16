pushd %~dp0
cd ..
set tgt=watcom
cmake --preset %tgt%
cmake --build --preset %tgt%-install
popd
