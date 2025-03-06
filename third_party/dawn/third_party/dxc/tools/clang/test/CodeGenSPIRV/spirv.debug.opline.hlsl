// RUN: %dxc -T ps_6_0 -E main -Zi -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.opline.hlsl

Texture2D    MyTexture;
SamplerState MySampler;

uint foo(uint val) {
  return val;
}

float4 main(uint val : A) : SV_Target {
  // CHECK:      OpLine [[file]] 18 24
  // CHECK-NEXT: OpLoad %uint %val
  // CHECK-NEXT: OpLine [[file]] 18 12
  // CHECK-NEXT: OpBitReverse
  uint a = reversebits(val);

  // CHECK:      OpLine [[file]] 22 16
  // CHECK-NEXT: OpLoad %uint %a
  uint b = foo(a);

  // CHECK:      OpLine [[file]] 26 14
  // CHECK-NEXT: OpLoad %type_2d_image %MyTexture
  float4 c = MyTexture.Sample(MySampler, float2(0.1, 0.2));

  // CHECK:      OpLine [[file]] 30 7
  // CHECK-NEXT: OpLoad %uint %val
  if (val > 10) {
    a = 5;
  } else {
    a = 6;
  }

  for (
  // CHECK:      OpLine [[file]] 39 7
  // CHECK-NEXT: OpStore %b %uint_0
      b = 0;
  // CHECK:      OpLine [[file]] 42 7
  // CHECK-NEXT: OpBranch %for_check
      b < 10;
  // CHECK:      OpLine [[file]] 45 9
  // CHECK-NEXT: OpLoad %uint %b
      ++b) {
    a += 1;
  }

  // CHECK:      OpLine [[file]] 51 12
  // CHECK-NEXT: OpLoad %uint %b
  while (--b > 0);

  do {
    c++;
  // CHECK:      OpLine [[file]] 57 12
  // CHECK-NEXT: OpAccessChain %_ptr_Function_float %c %int_0
  } while (c.x < 10);

// CHECK:      OpLine [[file]] 63 7
// CHECK-NEXT: OpAccessChain %_ptr_Function_float %c %int_0
// CHECK:      OpLine [[file]] 63 3
// CHECK-NEXT: OpStore %a
  a = c.x;

  return b * c;
}
