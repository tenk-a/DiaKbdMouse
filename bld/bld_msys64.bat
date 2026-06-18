pushd %~dp0
cd ..
set tgt=msys2-ucrt64
cmake --preset %tgt%
cmake --build --preset %tgt%-install
popd
