#!/bin/bash
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin
export PATH

rm -rf build_temp
mkdir build_temp
cd build_temp
cmake -DBUILD_SHARED_LIBS=ON ../../
make

if [ $? -ne 0 ]
then
    exit 1
fi

rm -rf ../../public/include/glog/*.*
cp -f build_temp/glog/*.h ../../public/include/glog/
cp -f ../src/glog/log_severity.h ../../public/include/glog/
cp -f logger/logger.h ../../public/include/glog/

rm -rf ../../public/so/libglog*
cp -f build_temp/libglog.so* ../../public/so/

exit 0

