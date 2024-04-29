@echo off
REM IMPORTANT: Only x64 is supported for now!
REM RESOURCE: https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically?view=msvc-170

REM IMPORTANT: -FC gets the full pathname. TODO(chowie): Check if all warnings are necessary
set CommonWarningFlags=  -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -wd4477
set CommonOptimiseFlags= -Od -Gm- -GR- -EHa- -Oi -fp:fast -fp:except-
set CommonDefinesFlags=  -DRUINENGLASS_INTERNAL=1 -DRUINENGLASS_SLOW=1 -DRUINENGLASS_WIN32=1
set CommonDebugFlags=    -FC -Z7 -Zo
set CommonCompilerFlags= -MTd -nologo %CommonWarningFlags% %CommonOptimiseFlags% %CommonDefinesFlags% %CommonDebugFlags%
REM IMPORTANT: -MTd -> MT for release mode

REM TODO(chowie): ole32.lib?
set CommonLibsFlags=     user32.lib gdi32.lib winmm.lib dwmapi.lib
set CommonLinkerFlags=   -incremental:no -opt:ref %CommonLibsFlags%

REM NOTE(chowie): Round-braces splits one-liners
IF NOT EXIST ..\..\build (
mkdir ..\..\build
)

pushd ..\..\build
REM NOTE(chowie): 'pwd' prints out the directory

REM TODO(chowie): Custom executable name RESOURCE: https://hero.handmade.network/forums/code-discussion/t/121-batch_script_notes
set ExecutableName= Ruinenglass

REM ForAllTestingGrounds
REM cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS ..\ruinenglass\code\test_gap_buffer.cpp /link %CommonLinkerFlags%

REM 64-bit build
cl %CommonCompilerFlags% ..\ruinenglass\code\win32_ruinenglass.cpp -Fmwin32_ruinenglass.map /link %CommonLinkerFlags%
popd
