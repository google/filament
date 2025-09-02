// RUN: %dxc -T cs_6_9 %s | FileCheck %s

RWByteAddressBuffer matrix_buffer;

// Test use of __builtin_VectorAccumulate in compute shader
// CHECK: define void @main()
// CHECK: call void @dx.op.vectorAccumulate.v2i32(i32 {{[0-9]+}}, <2 x i32> <i32 5, i32 5>, %dx.types.Handle {{%[0-9]+}}, i32 0)

[NumThreads(1,1,1)]
void main()
{
    vector<uint, 2> input_vector1 = 5;
    const uint matrix_offset = 0;

     __builtin_VectorAccumulate(input_vector1, matrix_buffer, matrix_offset);
}
