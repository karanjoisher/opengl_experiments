@echo off


REM +---------------------------------------- WARNING ------------------------------------------------------
REM |
REM | libcmt.lib: static CRT link library for a release build (/MT)
REM | libcmtd.lib: static CRT link library for a debug build (/MTd)
REM | msvcrt.lib: import library for the release DLL version of the CRT (/MD)
REM | msvcrtd.lib: import library for the debug DLL version of the CRT (/MDd)
REM | glew is using libcmt
REM | glfw is using msvcrt
REM | This causes a conflict so currenltly we are using NODEFAULT lib flag to ignore one of those libraries
REM | However it can cause some errors according to some sources
REM | To fix this build glew and glfw with same CRT option
REM +-------------------------------------------------------------------------------------------------------


REM +---------------- List of define switches for application -----------
REM | GL_CHECK_ERROS : Checks for errors after each GL call and logs them
REM | ASSERTS_ON : Enables assertions
REM +---------------------------------------------------------------------

if not exist build\. mkdir build
pushd build

set libdir=D:/workspace/libraries
set include_dirs=/I%libdir%/glfw_3_3_2_winx64_binaries/include/ /I%libdir%/glew_2_1_0_winx86x64_binaries/include/ /I%libdir%/imgui /I%libdir%/stb /I%libdir%/handmademath1_12_0

set libraries=%libdir%/glfw_3_3_2_winx64_binaries/lib-vc2019/glfw3.lib %libdir%/glew_2_1_0_winx86x64_binaries/lib/Release/x64/glew32s.lib %libdir%/imgui/build/imgui_glfw3_3_2_glew2_1_0.lib openGL32.lib kernel32.lib user32.lib Gdi32.lib shell32.lib 

set defines=-D_CRT_SECURE_NO_WARNINGS -DASSERTS_ON
set compiler_flags=-FC -MD -Gm- -nologo -GR- -EHa- -Oi  -W4 -wd4201 -wd4100 -wd4127 -wd4505 -wd4189 -fp:fast -Z7 -Od 
set linker_flags=/NODEFAULTLIB:LIBCMT

cl %defines% %compiler_flags% %include_dirs%  ..\main.cpp /link %linker_flags% %libraries% -incremental:no 

popd