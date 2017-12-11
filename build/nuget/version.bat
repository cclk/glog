cd "%~dp0"

@echo off

SET VER_MAJOR=1
SET VER_MINOR=3
SET VER_PATCH=5
SET OLD_VERSION=version-template

for /f "delims=" %%i in ('git rev-list --count HEAD') do (set REVISION=%%i)
for /f "delims=" %%i in ('git rev-parse --short HEAD') do (set REVISION_HASH=%%i)

if "%REVISION%" == "" (
	set REVISION=0
) 

SET NEW_VERSION=%VER_MAJOR%.%VER_MINOR%.%VER_PATCH%.%REVISION%

set originfile=glog-template.json
set outfile=glog.json

set utf8=65001
chcp %utf8%

(for /f "delims=" %%a in ('type %originfile%') do (
    set line=%%a   
    setlocal enabledelayedexpansion
    set line=!line:%OLD_VERSION%=%NEW_VERSION%!
    echo,!line!
    endlocal
))>%outfile%

set ansi=936
chcp %ansi%
