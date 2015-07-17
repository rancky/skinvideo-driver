@echo off
if "%DDKBUILDENV%"=="" goto no_setenv

rem Set CUSTOMTYPE=/DADD_FORMAT 
Set CUSTOMTYPE=/D_NOEFFECT

build -Zcg

goto exit

:no_setenv
echo Please set DDK build environment first
goto exit

:exit
Set CUSTOMTYPE=
