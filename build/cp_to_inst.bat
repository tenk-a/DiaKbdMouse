if not exist ..\inst mkdir ..\inst

if not exist ..\inst\win64 mkdir ..\inst\win64
copy /b vc142\exe_x64_Release\*.exe ..\inst\win64\
copy /b vc142\exe_x64_Release\*.dll ..\inst\win64\

if not exist ..\inst\win32 mkdir ..\inst\win32
copy /b vc142\exe_Win32_Release\*.exe ..\inst\win32\
copy /b vc142\exe_Win32_Release\*.dll ..\inst\win32\

if not exist ..\inst\win32_w2kXp mkdir ..\inst\win32_w2kXp
copy /b vc90\exe_Win32_Release\*.exe ..\inst\win32_w2kXp\
copy /b vc90\exe_Win32_Release\*.dll ..\inst\win32_w2kXp\
