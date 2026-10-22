// RUN: %dxc -T ps_6_2 -E main -fvk-use-dx-layout -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %_arr_float_uint_1 ArrayStride 16
// CHECK: OpMemberDecorate %type_buffer0 0 Offset 0
// CHECK: OpMemberDecorate %type_buffer0 1 Offset 16
// CHECK: OpMemberDecorate %type_buffer0 2 Offset 20
// CHECK: OpMemberDecorate %type_buffer0 3 Offset 24
// CHECK: OpMemberDecorate %type_buffer0 4 Offset 32
// CHECK: OpMemberDecorate %type_buffer0 5 Offset 40
// CHECK: OpMemberDecorate %type_buffer0 6 Offset 48
// CHECK: OpMemberDecorate %type_buffer0 7 Offset 52
// CHECK: OpMemberDecorate %type_buffer0 8 Offset 64
// CHECK: OpMemberDecorate %type_buffer0 9 Offset 72

// CHECK: %type_buffer0 = OpTypeStruct %half %_arr_float_uint_1 %half %float %v3half %double %half %v2float %v2float %float

cbuffer buffer0 {
  float16_t a;  // Offset:    0
  float b[1];   // Offset:   16
  float16_t c;  // Offset:   20
  float d;      // Offset:   24
  float16_t3 e; // Offset:   32
  double f;     // Offset:   40
  float16_t g;  // Offset:   48
  float2 h;     // Offset:   52
  float2 i;     // Offset:   64
  float end;    // Offset:   72
};

float4 main(float4 color : COLOR) : SV_TARGET
{
  color.x += end;
  return color;
}
