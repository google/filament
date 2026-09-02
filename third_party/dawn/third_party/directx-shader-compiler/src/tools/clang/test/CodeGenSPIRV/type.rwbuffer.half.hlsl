// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %type_buffer_image = OpTypeImage %float Buffer 2 0 0 2 Rgba16f
// CHECK: %_ptr_UniformConstant_type_buffer_image = OpTypePointer UniformConstant %type_buffer_image
// CHECK: %HalfBuffer = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant

RWBuffer<half4> HalfBuffer;

[numthreads(1,1,1)]
void main()
{
    HalfBuffer[0] = 1.0;
}

