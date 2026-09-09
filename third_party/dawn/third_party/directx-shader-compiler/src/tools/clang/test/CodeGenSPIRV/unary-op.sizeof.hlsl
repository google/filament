// RUN: %dxc -T ps_6_2 -E main -HV 2018 -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

struct PSInput
{
  int4 color : COLOR;
};
ByteAddressBuffer g_meshData[] : register(t0, space3);

struct EmptyStruct {};
struct SimpleStruct { int x; };

int3 main(PSInput input) : SV_TARGET
{
  int foo;

// CHECK: OpStore %foo %int_4
  foo = sizeof(int);
// CHECK: OpStore %foo %int_4
  foo = sizeof((int)0);
// CHECK: OpStore %foo %int_4
  foo = sizeof(int);
// CHECK: OpStore %foo %int_8
  foo = sizeof(int2);
// CHECK: OpStore %foo %int_16
  foo = sizeof(int2x2);
// CHECK: OpStore %foo %int_8
  foo = sizeof(int[2]);
// CHECK: OpStore %foo %int_4
  foo = sizeof(SimpleStruct);
// CHECK: OpStore %foo %int_0
  foo = sizeof(EmptyStruct);
// CHECK: OpStore %foo %int_12
  foo = sizeof(int16_t3[2]);
// CHECK: OpStore %foo %int_12
  foo = sizeof(half3[2]);
// CHECK: OpStore %foo %int_24
  foo = sizeof(int3[2]);
// CHECK: OpStore %foo %int_24
  foo = sizeof(float3[2]);
// CHECK: OpStore %foo %int_24
  foo = sizeof(bool3[2]);
// CHECK: OpStore %foo %int_48
  foo = sizeof(int64_t3[2]);
// CHECK: OpStore %foo %int_48
  foo = sizeof(double3[2]);
// CHECK: OpStore %foo %int_0
  foo = sizeof(EmptyStruct[2]);

  struct
  {
    int16_t i16;
    // 2-byte padding
    struct { float f32; } s; // Nested type
    struct {} _; // Zero-sized field.
  } complexStruct;
// CHECK: OpStore %foo %int_8
  foo = sizeof(complexStruct);

// CHECK: OpIMul %uint {{%[0-9]+}} %uint_12
  return g_meshData[input.color.x].Load3(input.color.y * sizeof(float3));
}
