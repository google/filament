// RUN: %dxc -T ps_6_0 -E main -O0 %s -spirv | FileCheck %s

// CHECK-NOT: %empty = OpVariable %_ptr_Uniform_type_empty Uniform
cbuffer empty {
};

float4 main(float2 uv : TEXCOORD) : SV_TARGET {
  return uv.xyxy;
}
