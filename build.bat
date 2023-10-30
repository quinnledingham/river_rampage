@echo off

pushd %CD%
IF NOT EXIST build mkdir build

set CFs= -MTd -nologo -Gm- -GR- -EHa- -Od -Oi -FC -Z7 /Isdl-vc\include /Iglad /Istb /Iqlib /DWINDOWS
set LFs= -incremental:no -opt:ref shell32.lib opengl32.lib sdl-vc\lib\x64\SDL2main.lib sdl-vc\lib\x64\SDL2.lib /subsystem:console

cl %CFs% game.cpp application.cpp /link %LFs% /out:build\river.exe

IF NOT EXIST build\SDL2.dll copy sdl-vc\lib\x64\SDL2.dll build