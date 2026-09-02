// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

// Test that non-const arithmetic are not optimized away

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);

[RootSignature("DescriptorTable(SRV(t0), SRV(t1))")]
float4 main() : SV_Target {
  // xHECK: %[[p_load:[0-9]+]] = load i32, i32*
  // xHECK-SAME: @dx.preserve.value
  // xHECK: %[[p:[0-9]+]] = trunc i32 %[[p_load]] to i1

  double x = 10;
  // select i1 %[[p]], double 1.000000e+01, %[[preserve_f64]]
  // CHECK: dx.nothing

  double y = x + 5;
  // xHECK: %[[a1:.+]] = fadd
  // select i1 %[[p]], double [[a1]], double [[a1]]
  // CHECK: dx.nothing

  double z = y * 2;
  // xHECK: %[[b1:.+]] = fmul
  // select i1 %[[p]], double [[b1]], double [[b1]]
  // CHECK: dx.nothing

  double w = z / 0.5;
  // xHECK: %[[c1:.+]] = fdiv
  // select i1 %[[p]], double [[c1]], double [[c1]]
  // CHECK: dx.nothing

  Texture2D tex = tex0; 
  // CHECK: load i32, i32*
  // CHECK-SAME: @dx.nothing

  // CHECK: br
  if (w >= 0) {
    tex = tex1;
    // CHECK: load i32, i32*
    // CHECK-SAME: @dx.nothing
    // CHECK: br
  }

  // CHECK: fadd
  // CHECK: fadd
  // CHECK: fadd
  // CHECK: fadd

  return tex.Load(0) + float4(x,y,z,w);
}

