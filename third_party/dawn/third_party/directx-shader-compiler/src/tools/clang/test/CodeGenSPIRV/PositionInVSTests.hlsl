// RUN: not %dxc -T vs_6_2 -E main -DTY=float -DARRAY=[4] -fcgl  %s -spirv 2>&1 | FileCheck %s
// RUN: not %dxc -T vs_6_2 -E main -DTY=double4 -fcgl  %s -spirv 2>&1 | FileCheck %s
// RUN: not %dxc -T vs_6_2 -E main -DTY=int4 -fcgl  %s -spirv 2>&1 | FileCheck %s
// RUN: not %dxc -T vs_6_2 -E main -DTY=float1x4 -fcgl  %s -spirv 2>&1 | FileCheck %s
// RUN: not %dxc -T vs_6_2 -E main -DTY=float3 -fcgl  %s -spirv 2>&1 | FileCheck %s
// RUN: not %dxc -T vs_6_2 -E main -DTY=InvalidType -fcgl  %s -spirv 2>&1 | FileCheck %s

// RUN: %dxc -T vs_6_2 -E main -DTY=ValidType -fcgl  %s -spirv | FileCheck %s --check-prefix=VALID_TY

// RUN: %dxc -T vs_6_2 -E main -DTY=float4 -fcgl  %s -spirv | FileCheck %s --check-prefix=VALID_FLOAT
// RUN: %dxc -T vs_6_2 -E main -DTY=min10float4 -fcgl  %s -spirv | FileCheck %s --check-prefix=VALID_FLOAT
// RUN: %dxc -T vs_6_2 -E main -DTY=min16float4 -fcgl  %s -spirv | FileCheck %s --check-prefix=VALID_FLOAT
// RUN: %dxc -T vs_6_2 -E main -DTY=half4 -fcgl  %s -spirv | FileCheck %s --check-prefix=VALID_FLOAT

// RUN: not %dxc -T vs_6_2 -E main -enable-16bit-types -DTY=half4 -fcgl  %s -spirv 2>&1 | FileCheck %s
// RUN: not %dxc -T vs_6_2 -E main -enable-16bit-types -DTY=min10float4 -fcgl  %s -spirv 2>&1 | FileCheck %s


// CHECK: error: SV_Position must be a 4-component 32-bit float vector or a composite which recursively contains only such a vector

// VALID_TY: %ValidType = OpTypeStruct %v4float
// VALID_TY: %output = OpTypeStruct %ValidType

// VALID_FLOAT: %output = OpTypeStruct %v4float




struct InvalidType {
  float3 x;
};

struct ValidType {
  float4 x;
};

#ifndef ARRAY
#define ARRAY
#endif

#define POSITION TY x ARRAY

struct output {
 POSITION;
};



output main() : SV_Position
{
    output result;
    return result;
}