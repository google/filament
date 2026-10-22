// RUN: %dxc -T cs_6_2 -E main -fvk-use-dx-layout -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

struct TestStruct0
{
    half first;
    double3 second;
    float last;
};

// CHECK: OpMemberDecorate %TestStruct0 0 Offset 0
// CHECK: OpMemberDecorate %TestStruct0 1 Offset 8
// CHECK: OpMemberDecorate %TestStruct0 2 Offset 32
// CHECK: OpDecorate %_runtimearr_TestStruct0 ArrayStride 40

struct TestStruct1
{
    half first;
    double3x2 second;
    float last;
};

// CHECK: OpMemberDecorate %TestStruct1 0 Offset 0
// CHECK: OpMemberDecorate %TestStruct1 1 Offset 8
// CHECK: OpMemberDecorate %TestStruct1 1 MatrixStride 24
// CHECK: OpMemberDecorate %TestStruct1 1 RowMajor
// CHECK: OpMemberDecorate %TestStruct1 2 Offset 56
// CHECK: OpDecorate %_runtimearr_TestStruct1 ArrayStride 64

struct InnerTestStruct2
{
    float a;
    double2x3 b[2];
    double3 c;
    half last;
};

struct TestStruct2
{
    float first;
    InnerTestStruct2 second;
    float last;
};

// CHECK: OpDecorate %_arr_mat2v3double_uint_2 ArrayStride 48
// CHECK: OpMemberDecorate %InnerTestStruct2 0 Offset 0
// CHECK: OpMemberDecorate %InnerTestStruct2 1 Offset 8
// CHECK: OpMemberDecorate %InnerTestStruct2 1 MatrixStride 16
// CHECK: OpMemberDecorate %InnerTestStruct2 1 RowMajor
// CHECK: OpMemberDecorate %InnerTestStruct2 2 Offset 104
// CHECK: OpMemberDecorate %InnerTestStruct2 3 Offset 128
// CHECK: OpMemberDecorate %TestStruct2 0 Offset 0
// CHECK: OpMemberDecorate %TestStruct2 1 Offset 8
// CHECK: OpMemberDecorate %TestStruct2 2 Offset 144
// CHECK: OpDecorate %_runtimearr_TestStruct2 ArrayStride 152

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
