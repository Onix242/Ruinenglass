@echo off
REM IMPORTANT(chowie): Only x64 is supported for now!
REM IMPORTANT(chowie): Check if all warnings are necessary
set CommonWarningFlags=  -diagnostics:column -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -wd4477
set CommonOptimiseFlags= -Od -Gm- -GR- -EHa- -Oi -fp:except-
set CommonDefinesFlags=  -DRUINENGLASS_INTERNAL=1 -DRUINENGLASS_SLOW=1 -DRUINENGLASS_WIN32=1
set CommonDebugFlags=    -FC -Z7 -Zo -Fm
set CommonCompilerFlags= -MTd -nologo %CommonWarningFlags% %CommonOptimiseFlags% %CommonDefinesFlags% %CommonDebugFlags%
REM IMPORTANT: -MTd -> MT for release mode

REM TODO(chowie): Remove dwmapi.lib after OpenGL!
set CommonLibsFlags=      user32.lib gdi32.lib winmm.lib dwmapi.lib
set CommonLinkerShared=  -link -CETCOMPAT -incremental:no -opt:ref
set CommonLinkerFlags=    %CommonLinkerShared% %CommonLibsFlags%

set GameDLLExports=      -PDB:ruinenglass_%random%.pdb -EXPORT:GameUpdateAndRender -EXPORT:GameGetSoundSamples
set GameDLLFlags=        -LD %CommonLinkerShared% %GameDLLExports%

IF NOT EXIST ..\..\build (
mkdir ..\..\build
)
pushd ..\..\build

del *.pdb > NUL 2> NUL
echo WAITING FOR PDB > lock.tmp

REM TODO(chowie): Custom executable name
REM RESOURCE(theinternetftw): https://hero.handmade.network/forums/code-discussion/t/121-batch_script_notes
set ExecutableName= Ruinenglass

REM ForAllTestingGrounds
REM cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS ..\ruinenglass\code\test_gap_buffer.cpp %CommonLinkerFlags%

REM GAME
cl %CommonCompilerFlags% ..\ruinenglass\code\ruinenglass.cpp       %GameDLLFlags%
del lock.tmp
cl %CommonCompilerFlags% ..\ruinenglass\code\win32_ruinenglass.cpp %CommonLinkerFlags%
popd

REM
REM Batch Notes
REM

REM Testing on new comp
REM For observing how long everything takes, use -d1reportTime

REM NOTE(chowie): According to martins, fp:fast generates broken instructions
REM RESOURCE(msdn): https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically?view=msvc-170
REM RESOURCE(msdn): https://learn.microsoft.com/en-us/cpp/c-runtime-library/potential-errors-passing-crt-objects-across-dll-boundaries?view=msvc-170&redirectedfrom=MSDN
REM NOTE(chowie): Round-braces splits one-liners
REM NOTE(chowie): 'pwd' prints out the directory
REM IMPORTANT(chowie): -FC gets the full pathname.

REM NOTE(chowie): CETCOMPAT = Shadow Stack protection for buffer
REM overflow. Separates buffer's address spacing as to not buffer
REM overflow for very little downside and security risk. Recommended
REM by JBlow! Like GCC/Clang SafeStack/ShadowStack, -mshstk, and
REM -fsanitised-safe-stack.

