@echo off

REM IMPORTANT: -FC to get the full path name!
set CommonCompilerFlags= -Od -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -wd4477 -FC -Z7
set CommonCompilerFlags= -DRUINENGLASS_INTERNAL=1 -DRUINENGLASS_SLOW=1 -DRUINENGLASS_WIN32=1 %CommonCompilerFlags%
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib
REM ole32.lib
REM set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build
REM To print out the directory use, 'pwd'

REM 64-bit build
cl %CommonCompilerFlags% ..\ruinenglass\code\win32_ruinenglass.cpp -Fmwin32_ruinenglass.map /link %CommonLinkerFlags%
popd
