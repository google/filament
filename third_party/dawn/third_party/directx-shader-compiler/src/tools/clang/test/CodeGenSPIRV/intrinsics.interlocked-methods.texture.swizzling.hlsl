// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %type_3d_image = OpTypeImage %uint 3D 2 0 0 2 R32ui
// CHECK:        %target = OpVariable %_ptr_UniformConstant_type_3d_image UniformConstant
RWTexture3D<uint> target : register(u1);

struct VS_OUTPUT
{
  float4 Pos : SV_Position;
};

void main(VS_OUTPUT input)
{
  uint3 index = uint3(0,0,0);

// CHECK: [[ptr:%[0-9]+]] = OpImageTexelPointer %_ptr_Image_uint %target {{%[0-9]+}} %uint_0
// CHECK-NEXT:           OpAtomicIAdd %uint [[ptr]] %uint_1 %uint_0 %uint_1
  InterlockedAdd(target[index].r, 1);
}
