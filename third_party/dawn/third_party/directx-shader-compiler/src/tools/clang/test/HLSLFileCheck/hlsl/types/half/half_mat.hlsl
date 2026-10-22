// RUN: %dxc -enable-16bit-types -T lib_6_3 -default-linkage external %s | FileCheck %s

// CHECK: %class.matrix.half.4.3 = type { [4 x <3 x half>] }

cbuffer CB : register(b0)
{
  row_major float16_t4x3 mat;
};

float3 foo() {
  return mat[0];
}
