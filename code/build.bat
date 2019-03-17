@echo off

ECHO === Building program ===

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM -Zi is for showing warning stuff that annoyes me
REM 4mb of stack size -F4194304
REM -Z7

set program_name=game
set includes="W:\game_app\extra_incs"
set libs="W:\game_app\extra_libs"

set code_to_compile=../code/*.cpp ../code/glad/glad.c
set debug_flags=-FC -Zi -Fmgame_app.map
set code_macros=-D_CRT_SECURE_NO_WARNINGS -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1

REM no-logo
set ignored_warnings=-wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -wd4244 -wd4018 -wd4706 -wd4267 -wd4245 -wd4996 -wd4172 -wd4049 -wd4098 -wd4217 -wd4005
set common_compiler_flags=-MT -Gm -GR- -EHa- -Oi -WX -W4 %ignored_warnings% %code_macros% %debug_flags%
set common_linker_flags=-opt:ref user32.lib shell32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib glm_static_d.lib glfw3.lib

cl %common_compiler_flags% /Fe%program_name% %code_to_compile% /I %includes% /link /LIBPATH:%libs% %common_linker_flags%

popd