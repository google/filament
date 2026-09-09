// RUN: %dxc /Od /T vs_6_0 /E main %s | FileCheck %s

// Regression check for a case with /Od where there is a static variable struct
// write before an alloca. As part of the algorithm to replace all uses of the
// GV before initialization with 0's, the basic block is split where the first
// memcpy occurs. This may cause alloca's to get stuck in a non-emtry block and
// be missed by lowering transformations, such as in this example, an alloca of
// <16 x float> sticks around.

// CHECK: void @main

float4x4 make(float4 a, float4 b, float4 c, float4 d)
{
 float4x4 mat;
 mat._11_21_31_41 = a;
 mat._12_22_32_42 = b;
 mat._13_23_33_43 = c;
 mat._14_24_34_44 = d;
 return mat;
}
struct A {
 float2 foo;
};

static A glob_a;

void main()
{
 A a;
 glob_a = a;
 float3 b = float3(make(0, 1, 2, float4(0,0,0,1))[3].xyz);
}