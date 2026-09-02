// RUN: %dxc -T cs_6_2 -E main -fvk-use-dx-layout -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

struct TestStruct0
{
    double Double;
    float3 Float;
};

struct TestStruct1
{
    double Double;
    float Float;
};

struct TestStruct2
{
    double Double[2];
    float Float;
};

struct TestStruct3
{
    double Double[2];
    float3 Float;
};

struct TestStruct4
{
    float a;
    half b;
};

struct TestStruct5
{
    float a;
    half3 b;
};

struct TestStruct6
{
    float2 a;
    half3 b;
};

RWTexture2D<float4> Result;

// CHECK: OpMemberDecorate %TestStruct0 0 Offset 0
// CHECK: OpMemberDecorate %TestStruct0 1 Offset 8
// CHECK: OpDecorate %_runtimearr_TestStruct0 ArrayStride 24
StructuredBuffer<TestStruct0> testSB0;

// CHECK: OpMemberDecorate %TestStruct1 0 Offset 0
// CHECK: OpMemberDecorate %TestStruct1 1 Offset 8
// CHECK: OpDecorate %_runtimearr_TestStruct1 ArrayStride 16
StructuredBuffer<TestStruct1> testSB1;

// CHECK: OpMemberDecorate %TestStruct2 0 Offset 0
// CHECK: OpMemberDecorate %TestStruct2 1 Offset 16
// CHECK: OpDecorate %_runtimearr_TestStruct2 ArrayStride 24
StructuredBuffer<TestStruct2> testSB2;

// CHECK: OpMemberDecorate %TestStruct3 0 Offset 0
// CHECK: OpMemberDecorate %TestStruct3 1 Offset 16
// CHECK: OpDecorate %_runtimearr_TestStruct3 ArrayStride 32
StructuredBuffer<TestStruct3> testSB3;

// CHECK: OpMemberDecorate %TestStruct4 0 Offset 0
// CHECK: OpMemberDecorate %TestStruct4 1 Offset 4
// CHECK: OpDecorate %_runtimearr_TestStruct4 ArrayStride 8
StructuredBuffer<TestStruct4> testSB4;

// CHECK: OpMemberDecorate %TestStruct5 0 Offset 0
// CHECK: OpMemberDecorate %TestStruct5 1 Offset 4
// CHECK: OpDecorate %_runtimearr_TestStruct5 ArrayStride 12
StructuredBuffer<TestStruct5> testSB5;

// CHECK: OpMemberDecorate %TestStruct6 0 Offset 0
// CHECK: OpMemberDecorate %TestStruct6 1 Offset 8
// CHECK: OpDecorate %_runtimearr_TestStruct6 ArrayStride 16
StructuredBuffer<TestStruct6> testSB6;

[numthreads(8,8,2)]
void main (uint3 id : SV_DispatchThreadID)
{
    TestStruct0 temp0 = testSB0[id.z];
    Result[id.xy] = float(temp0.Double).xxxx;

    TestStruct1 temp1 = testSB1[id.x];
    Result[id.zy] = float(temp1.Double).xxxx;
}

