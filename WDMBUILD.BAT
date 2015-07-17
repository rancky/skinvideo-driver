@echo off
if %1.==. goto usage
if not exist %1\bin\setenv.bat goto usage

pushd ..
call %1\bin\setenv %1 %2 %3
popd

Set CUSTOMTYPE=/D%4

build -Zcg

goto exit

:usage
echo usage wdmbuild ddk_dir [fre/chk] [WIN7/x64] [ADD_FORMAT/_NOEFFECT]
echo eg wdmbuild C:\WINDDK\7600.16385.0 chk WIN7 ADD_FORMAT
echo eg wdmbuild C:\WINDDK\7600.16385.0 fre x64 _NOEFFECT

:exit
