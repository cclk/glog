cd "%~dp0"
call "%~dp0\..\build-win32.bat"

cd "%~dp0"
set builddir=%cd%
echo %builddir%

rd /q /s build
md build\include
md build\include\glog
md build\include\Logger

xcopy /E ..\build_temp\glog\*.*     build\include\glog\
copy ..\..\src\glog\log_severity.h  build\include\glog\log_severity.h
copy ..\logger\logger.h             build\include\glog\logger.h
copy Logger.h                       build\include\Logger\Logger.h


"%PROGRAMFILES(x86)%\MSBuild\14.0\Bin\MSbuild.exe" "..\build_temp\glog.sln" /t:glog /p:PlatformToolset=v140 /p:configuration=Debug    /p:OutDir=%builddir%\build\v140\dynamic\bin\debug\
"%PROGRAMFILES(x86)%\MSBuild\14.0\Bin\MSbuild.exe" "..\build_temp\glog.sln" /t:glog /p:PlatformToolset=v140 /p:configuration=Release  /p:OutDir=%builddir%\build\v140\dynamic\bin\release\

copy ..\build_temp\Debug\glog.lib   %builddir%\build\v140\dynamic\bin\debug\glog.lib
copy ..\build_temp\Debug\glog.pdb   %builddir%\build\v140\dynamic\bin\debug\glog.pdb
copy ..\build_temp\Release\glog.lib %builddir%\build\v140\dynamic\bin\release\glog.lib
copy ..\build_temp\Release\glog.pdb %builddir%\build\v140\dynamic\bin\release\glog.pdb

::::::::::::::::::::::::::::::::::::::::::::nuget build
cd "%~dp0"
del /q /s *.nupkg
del /q /s *.targets
del /q /s *.nuspec
del /q /s *.xml
del /q /s glog.json

cd "%~dp0"
call version.bat
create_nuget glog.json

echo "next upload to cvte nuget server"
nuget push *.nupkg  -source http://iipdev.gz.cvte.cn:8080 CVTE

cd "%~dp0"


