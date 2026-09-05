// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure only 2 cbufferLoadLegacy.
// CHECK:call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32 59, %dx.types.Handle %{{.*}}, i32 0)
// CHECK:call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32 59, %dx.types.Handle %{{.*}}, i32 1)
// CHECK-NOT:call %dx.types.CBufRet.f64

double4 g;

float main(int2 i:I) : SV_Target {
  return g[i.x]+g[i.y];
}