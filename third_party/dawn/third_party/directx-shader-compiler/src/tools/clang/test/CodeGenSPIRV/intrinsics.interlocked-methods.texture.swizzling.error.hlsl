// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

RWTexture3D<uint3> target : register(u1);

struct VS_OUTPUT
{
  float4 Pos : SV_Position;
};

void main(VS_OUTPUT input)
{
  uint3 index = uint3(0,0,0);
  InterlockedAdd(target[index].r, 1);
}

// CHECK: :13:18: error: Interlocked operation for texture buffer whose result type is non-scalar type is not allowed
