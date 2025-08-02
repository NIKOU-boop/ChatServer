#!/bin/bash
set -x

# 删除build目录下的所有内容
rm -rf "$(pwd)/build"/*

# 进入build目录并执行编译
cd "$(pwd)/build" &&
cmake .. &&
make

