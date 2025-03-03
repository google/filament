// RUN: %clang_cc1 %s -triple nvptx-unknown-unknown -fcuda-is-device -emit-llvm -o - | FileCheck %s

#include "Inputs/cuda.h"

// CHECK-LABEL: define void @device_function
extern "C"
__device__ void device_function() {}

// CHECK-LABEL: define void @global_function
extern "C"
__global__ void global_function() {
  // CHECK: call void @device_function
  device_function();
}

// CHECK: !{{[0-9]+}} = !{void ()* @global_function, !"kernel", i32 1}
