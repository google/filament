// RUN: %clang_cc1 -emit-llvm %s -o - -fcuda-is-device -triple nvptx-unknown-unknown | FileCheck %s

// Verifies Clang emits correct address spaces and addrspacecast instructions
// for CUDA code.

#include "Inputs/cuda.h"

// CHECK: @i = addrspace(1) global
__device__ int i;

// CHECK: @j = addrspace(4) global
__constant__ int j;

// CHECK: @k = addrspace(3) global
__shared__ int k;

struct MyStruct {
  int data1;
  int data2;
};

// CHECK: @_ZZ5func0vE1a = internal addrspace(3) global %struct.MyStruct zeroinitializer
// CHECK: @_ZZ5func1vE1a = internal addrspace(3) global float 0.000000e+00
// CHECK: @_ZZ5func2vE1a = internal addrspace(3) global [256 x float] zeroinitializer
// CHECK: @_ZZ5func3vE1a = internal addrspace(3) global float 0.000000e+00
// CHECK: @_ZZ5func4vE1a = internal addrspace(3) global float 0.000000e+00
// CHECK: @b = addrspace(3) global float 0.000000e+00

__device__ void foo() {
  // CHECK: load i32, i32* addrspacecast (i32 addrspace(1)* @i to i32*)
  i++;

  // CHECK: load i32, i32* addrspacecast (i32 addrspace(4)* @j to i32*)
  j++;

  // CHECK: load i32, i32* addrspacecast (i32 addrspace(3)* @k to i32*)
  k++;

  static int li;
  // CHECK: load i32, i32* addrspacecast (i32 addrspace(1)* @_ZZ3foovE2li to i32*)
  li++;

  __constant__ int lj;
  // CHECK: load i32, i32* addrspacecast (i32 addrspace(4)* @_ZZ3foovE2lj to i32*)
  lj++;

  __shared__ int lk;
  // CHECK: load i32, i32* addrspacecast (i32 addrspace(3)* @_ZZ3foovE2lk to i32*)
  lk++;
}

__device__ void func0() {
  __shared__ MyStruct a;
  MyStruct *ap = &a; // composite type
  ap->data1 = 1;
  ap->data2 = 2;
}
// CHECK: define void @_Z5func0v()
// CHECK: store %struct.MyStruct* addrspacecast (%struct.MyStruct addrspace(3)* @_ZZ5func0vE1a to %struct.MyStruct*), %struct.MyStruct** %ap

__device__ void callee(float *ap) {
  *ap = 1.0f;
}

__device__ void func1() {
  __shared__ float a;
  callee(&a); // implicit cast from parameters
}
// CHECK: define void @_Z5func1v()
// CHECK: call void @_Z6calleePf(float* addrspacecast (float addrspace(3)* @_ZZ5func1vE1a to float*))

__device__ void func2() {
  __shared__ float a[256];
  float *ap = &a[128]; // implicit cast from a decayed array
  *ap = 1.0f;
}
// CHECK: define void @_Z5func2v()
// CHECK: store float* getelementptr inbounds ([256 x float], [256 x float]* addrspacecast ([256 x float] addrspace(3)* @_ZZ5func2vE1a to [256 x float]*), i32 0, i32 128), float** %ap

__device__ void func3() {
  __shared__ float a;
  float *ap = reinterpret_cast<float *>(&a); // explicit cast
  *ap = 1.0f;
}
// CHECK: define void @_Z5func3v()
// CHECK: store float* addrspacecast (float addrspace(3)* @_ZZ5func3vE1a to float*), float** %ap

__device__ void func4() {
  __shared__ float a;
  float *ap = (float *)&a; // explicit c-style cast
  *ap = 1.0f;
}
// CHECK: define void @_Z5func4v()
// CHECK: store float* addrspacecast (float addrspace(3)* @_ZZ5func4vE1a to float*), float** %ap

__shared__ float b;

__device__ float *func5() {
  return &b; // implicit cast from a return value
}
// CHECK: define float* @_Z5func5v()
// CHECK: ret float* addrspacecast (float addrspace(3)* @b to float*)

struct StructWithCtor {
  __device__ StructWithCtor(): data(1) {}
  __device__ StructWithCtor(const StructWithCtor &second): data(second.data) {}
  __device__ int getData() { return data; }
  int data;
};

__device__ int construct_shared_struct() {
// CHECK-LABEL: define i32 @_Z23construct_shared_structv()
  __shared__ StructWithCtor s;
// CHECK: call void @_ZN14StructWithCtorC1Ev(%struct.StructWithCtor* addrspacecast (%struct.StructWithCtor addrspace(3)* @_ZZ23construct_shared_structvE1s to %struct.StructWithCtor*))
  __shared__ StructWithCtor t(s);
// CHECK: call void @_ZN14StructWithCtorC1ERKS_(%struct.StructWithCtor* addrspacecast (%struct.StructWithCtor addrspace(3)* @_ZZ23construct_shared_structvE1t to %struct.StructWithCtor*), %struct.StructWithCtor* dereferenceable(4) addrspacecast (%struct.StructWithCtor addrspace(3)* @_ZZ23construct_shared_structvE1s to %struct.StructWithCtor*))
  return t.getData();
// CHECK: call i32 @_ZN14StructWithCtor7getDataEv(%struct.StructWithCtor* addrspacecast (%struct.StructWithCtor addrspace(3)* @_ZZ23construct_shared_structvE1t to %struct.StructWithCtor*))
}
