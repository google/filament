// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s
// Test failure expected when run with 19041 SDK DXIL.dll

// CHECK:   %[[RayTCurrent:RayTCurrent|[0-9]+]] = call float @dx.op.rayTCurrent.f32(i32 154)
// CHECK:   %[[WaveActiveOp:WaveActiveOp|[0-9]+]] = call float @dx.op.waveActiveOp.f32(i32 119, float %[[RayTCurrent]], i8 2, i8 0)
// CHECK:   call i1 @dx.op.reportHit.struct.MyAttributes(i32 158, float %[[WaveActiveOp]], i32 0,

struct MyAttributes {
  float2 bary;
  uint id;
};

[shader("intersection")]
void intersection1()
{
  float hitT = RayTCurrent();
  MyAttributes attr = (MyAttributes)0;
  bool bReported = ReportHit(WaveActiveMin(hitT), 0, attr);
}
