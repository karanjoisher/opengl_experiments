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

REM +---------------- BUILD IMGUI (OpenGL loaded static) --------------------------
REM | set imgui_source=D:/workspace/libraries/imgui
REM | cl -DGLEW_STATIC /ID:/workspace/libraries/glew_2_1_0_winx86x64_binaries/include/ /ID:/workspace/libraries/glfw_3_3_2_winx64_binaries/include/ %imgui_source%\imgui_impl_glfw.cpp %imgui_source%\imgui_impl_opengl3.cpp %imgui_source%\imgui*.cpp
REM +------------------------------------------------------------------------------  

REM +---------------- BUILD APPLICATION ---------------------
set include_dirs=/ID:/workspace/libraries/glfw_3_3_2_winx64_binaries/include/ /ID:/workspace/libraries/glew_2_1_0_winx86x64_binaries/include/ /ID:/workspace/libraries/glm_0_9_9_8 /ID:/workspace/libraries/imgui

set libraries=D:/workspace/libraries/glfw_3_3_2_winx64_binaries/lib-vc2019/glfw3.lib D:/workspace/libraries/glew_2_1_0_winx86x64_binaries/lib/Release/x64/glew32s.lib openGL32.lib kernel32.lib user32.lib Gdi32.lib shell32.lib 

set compiler_flags=-FC -MD -Gm- -nologo -GR- -EHa- -Oi  -W4 -wd4201 -wd4100 -wd4127 -wd4505 -wd4189 -fp:fast -Z7 -Od 
set linker_flags=/NODEFAULTLIB:LIBCMT
set defines=-D_CRT_SECURE_NO_WARNINGS

cl %defines% %compiler_flags% %include_dirs%  ..\main.cpp /link %linker_flags% %libraries% imgui_impl_glfw.obj imgui_impl_opengl3.obj imgui_widgets.obj imgui_draw.obj imgui.obj imgui_demo.obj -incremental:no 
REM +--------------------------------------------------------

popd