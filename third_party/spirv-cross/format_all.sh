#!/bin/bash
# Copyright 2016-2021 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

#for file in spirv_*.{cpp,hpp} include/spirv_cross/*.{hpp,h} samples/cpp/*.cpp main.cpp
for file in spirv_*.{cpp,hpp} main.cpp
do
    echo "Formatting file: $file ..."
    clang-format -style=file -i $file
done
