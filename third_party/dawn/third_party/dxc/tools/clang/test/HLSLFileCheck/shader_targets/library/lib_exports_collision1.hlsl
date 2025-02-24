// RUN: %dxc -auto-binding-space 13 -exports Foo=VSMain;Foo=VSMainDup;Foo=\01?VSMain@@YA?AV?$vector@M$03@@V?$vector@H$02@@@Z;Foo=\01?VSMainDup@@YA?AV?$vector@M$03@@V?$vector@H$02@@@Z;RayGen;RayGen=fn -T lib_6_3 %s | FileCheck %s

// Verify export collision errors
// CHECK: error: Export name collides with another export: \01?Foo{{[@$?.A-Za-z0-9_]+}}
// CHECK: error: Export name collides with another export: \01?RayGen{{[@$?.A-Za-z0-9_]+}}
// CHECK: error: Export name collides with another export: Foo

Buffer<int> T0;

Texture2D<float4> T1;

struct Foo { uint u; float f; };
StructuredBuffer<Foo> T2;

RWByteAddressBuffer U0;

[shader("vertex")]
float4 VSMain(int3 coord : COORD) : SV_Position {
  return T1.Load(coord);
}

[shader("vertex")]
float4 VSMainDup(int3 coord : COORD) : SV_Position {
  return T1.Load(coord);
}

[shader("pixel")]
float4 PSMain(int idx : INDEX) : SV_Target {
  return T2[T0.Load(idx)].f;
}

void fn() {
}

[shader("raygeneration")]
void RayGen() {
  uint2 dim = DispatchRaysDimensions();
  uint2 idx = DispatchRaysIndex();
  U0.Store(idx.y * dim.x * 4 + idx.x * 4, idx.x ^ idx.y);
}
