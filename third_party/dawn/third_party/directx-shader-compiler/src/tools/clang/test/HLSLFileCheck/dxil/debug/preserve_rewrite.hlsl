// RUN: %dxc -E main -T ps_6_0 %s -Od /Zi | FileCheck %s

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);

[RootSignature("DescriptorTable(SRV(t0), SRV(t1))")]
float main() : SV_Target {
  // xHECK: %[[p_load:[0-9]+]] = load i32, i32*
  // xHECK-SAME: @dx.preserve.value
  // xHECK: %[[p:[0-9]+]] = trunc i32 %[[p_load]] to i1

  int x = 10;
  // xHECK: %[[x1:.+]] = select i1 %[[p]], i32 10, i32 10
  // CHECK: dbg.value(metadata i32 10, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"x"
  // CHECK: dx.nothing

  x = 6;
  // xHECK: %[[x2:.+]] = select i1 %[[p]], i32 %[[x1]], i32 6
  // CHECK: dbg.value(metadata i32 6, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"x"
  // CHECK: dx.nothing

  x = 10;
  // xHECK: %[[x3:.+]] = select i1 %[[p]], i32 %[[x2]], i32 10
  // CHECK: dbg.value(metadata i32 10, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"x"
  // CHECK: dx.nothing

  x = 40;
  // xHECK: %[[x4:.+]] = select i1 %[[p]], i32 %[[x3]], i32 40
  // CHECK: dbg.value(metadata i32 40, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"x"
  // CHECK: dx.nothing

  x = 80;
  // xHECK: %[[x5:.+]] = select i1 %[[p]], i32 %[[x4]], i32 80
  // CHECK: dbg.value(metadata i32 80, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"x"

  x = x * 5;
  // xHECK: %[[x6:.+]] = mul 
  // xHECK-SAME: %[[x5]]
  // CHECK: dbg.value(metadata i32 400, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"x"

  // CHECK: dx.nothing
  return x;
}

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

