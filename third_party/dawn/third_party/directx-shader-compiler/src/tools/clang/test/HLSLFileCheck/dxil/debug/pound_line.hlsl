// RUN: %dxc -Zi -E main -T ps_6_0 %s | FileCheck %s

// CHECK-NOT: !{{[0-9]+}} = !DIFile(filename: "FileThatDoesntExist.hlsl"
// CHECK-NOT: !{{[0-9]+}} = !DILocation(line: 32
// CHECK-NOT: !{{[0-9]+}} = !DILocation(line: 33

#line 30 "FileThatDoesntExist.hlsl"
[RootSignature("")]
float main(int a : TEXCOORD) : SV_Target {
  float x = 0;
  return x;
}


