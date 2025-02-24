// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK-DAG: textureLoad.f32({{.*}}, i32 undef, i32 undef, i32 undef)
// CHECK-DAG: textureLoad.f32({{.*}}, i32 -5, i32 7, i32 undef)
// CHECK-DAG: textureLoad.f32({{.*}}, i32 0, i32 0, i32 undef)

Texture2DMS<float3> srv1 : register(t3);

float3 main(int2 a : A, int c : C, int2 b : B) : SV_Target
{
  uint status;
  uint2 offset = uint2(-5, 7);
  float3 r = 0;
  r += srv1.Load(a, c);
  r += srv1[b];
  r += srv1.Load(a, c, offset);
  r += srv1.Load(a, c, offset, status); r += status;
  r += srv1.Load(a, c, uint2(0,0), status); r += status;
  // TODO: enable this.
  // r += srv1[2][b];
  return r;
}
