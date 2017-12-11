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

git rev-list HEAD | wc -l > version

exit 0

