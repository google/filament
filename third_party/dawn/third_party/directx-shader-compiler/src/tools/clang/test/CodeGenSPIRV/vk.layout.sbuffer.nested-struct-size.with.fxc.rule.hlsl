// RUN: %dxc -T cs_6_2 -E main -fvk-use-dx-layout -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

struct InnerTestStruct0
{
    float a;
    double3 b[2];
    half last;
};

struct TestStruct0
{
    half first;
    InnerTestStruct0 second;
    float last;
};

// CHECK: OpMemberDecorate %InnerTestStruct0 0 Offset 0
// CHECK: OpMemberDecorate %InnerTestStruct0 1 Offset 8
// CHECK: OpMemberDecorate %InnerTestStruct0 2 Offset 56
// CHECK: OpMemberDecorate %TestStruct0 0 Offset 0
// CHECK: OpMemberDecorate %TestStruct0 1 Offset 8
// CHECK: OpMemberDecorate %TestStruct0 2 Offset 72
// CHECK: OpDecorate %_runtimearr_TestStruct0 ArrayStride 80

struct InnerTestStruct1
{
    float a;
    double3 b[2];
    half last;
};

struct TestStruct1
{
    InnerTestStruct1 first;
    float last;
};

// CHECK: OpMemberDecorate %InnerTestStruct1 0 Offset 0
// CHECK: OpMemberDecorate %InnerTestStruct1 1 Offset 8
// CHECK: OpMemberDecorate %InnerTestStruct1 2 Offset 56
// CHECK: OpMemberDecorate %TestStruct1 0 Offset 0
// CHECK: OpMemberDecorate %TestStruct1 1 Offset 64
// CHECK: OpDecorate %_runtimearr_TestStruct1 ArrayStride 72

struct InnerTestStruct2
{
    float a;
    double3 b[2];
    half last;
};

struct TestStruct2
{
    float first;
    InnerTestStruct2 second;
    float last;
};

// CHECK: OpMemberDecorate %InnerTestStruct2 0 Offset 0
// CHECK: OpMemberDecorate %InnerTestStruct2 1 Offset 8
// CHECK: OpMemberDecorate %InnerTestStruct2 2 Offset 56
// CHECK: OpMemberDecorate %TestStruct2 0 Offset 0
// CHECK: OpMemberDecorate %TestStruct2 1 Offset 8
// CHECK: OpMemberDecorate %TestStruct2 2 Offset 72
// CHECK: OpDecorate %_runtimearr_TestStruct2 ArrayStride 80

RWTexture2D<float4> Result;

StructuredBuffer<TestStruct0> testSB0;
StructuredBuffer<TestStruct1> testSB1;
StructuredBuffer<TestStruct2> testSB2;

[numthreads(8,8,2)]
void main (uint3 id : SV_DispatchThreadID)
{
    TestStruct0 temp0 = testSB0[id.z];
    Result[id.xy] = float(temp0.last).xxxx;

    TestStruct1 temp1 = testSB1[id.z];
    Result[id.xy] = float(temp1.last).xxxx;

    TestStruct2 temp2 = testSB2[id.z];
    Result[id.xy] = float(temp2.last).xxxx;
}
