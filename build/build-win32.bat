cd /d "%~dp0"

rd /q /s build_temp
mkdir build_temp
cd build_temp

cmake -G "Visual Studio 14" -T "v140_xp" -DBUILD_SHARED_LIBS=ON ../../

cd ..