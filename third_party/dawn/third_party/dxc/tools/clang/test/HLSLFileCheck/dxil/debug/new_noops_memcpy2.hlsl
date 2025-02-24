// RUN: %dxc -E main -T ps_6_0 %s -Od /Zi | FileCheck %s

typedef float4 foo;

[RootSignature("")]
float4 main(float4 color : COLOR) : SV_Target {

  foo f1 = color;
  // CHECK: dx.nothing.a, i32 0, i32 0), !dbg !{{[0-9]+}} ; line:8

  foo f2[2];

  f2[0].y = color.y;
  // CHECK: dx.nothing.a, i32 0, i32 0), !dbg !{{[0-9]+}} ; line:13

  f2[1].w = color.w;
  // CHECK: dx.nothing.a, i32 0, i32 0), !dbg !{{[0-9]+}} ; line:16

  foo arrayOfFooArray[2][2];

  arrayOfFooArray[0] = f2; // This is a memcpy
  // CHECK: dx.nothing.a, i32 0, i32 0), !dbg !{{[0-9]+}} ; line:21

  arrayOfFooArray[1] = f2; // This is a memcpy
  // CHECK: dx.nothing.a, i32 0, i32 0), !dbg !{{[0-9]+}} ; line:24

  return float4(f1.x + f1.z, + f2[0].y + f2[1].w, arrayOfFooArray[0][0].y, arrayOfFooArray[1][1].w) + color;
  // CHECK: dx.nothing.a, i32 0, i32 0), !dbg !{{[0-9]+}} ; line:27
}

