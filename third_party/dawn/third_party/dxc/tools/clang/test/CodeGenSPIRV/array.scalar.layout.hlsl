// RUN: %dxc -T cs_6_2 -E main %s -fvk-use-scalar-layout -spirv | FileCheck %s

// Check that the array stride and offsets are corrects. The uint64_t has alignment
// 8 and the struct has size 12. So the stride should be the smallest multiple of 8
// greater than or equal to 12, which is 16.

// CHECK-DAG: OpMemberDecorate %Data 0 Offset 0
// CHECK-DAG: OpMemberDecorate %Data 1 Offset 8
// CHECK-DAG: OpDecorate %_runtimearr_Data ArrayStride 16
// CHECK-DAG: OpMemberDecorate %type_RWStructuredBuffer_Data 0 Offset 0
struct Data {
    uint64_t y;
    uint x;
};
RWStructuredBuffer<Data> buffer;

[numthreads(1, 1, 1)]
void main()
{
    buffer[0].x = 5;
}
