// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK: define void [[intersection1:@"\\01\?intersection1@[^\"]+"]]() #0 {
// CHECK:   [[rayTCurrent:%[^ ]+]] = call float @dx.op.rayTCurrent.f32(i32 154)
// CHECK:   call i1 @dx.op.reportHit.struct.MyAttributes(i32 158, float [[rayTCurrent]], i32 0, %struct.MyAttributes* nonnull {{.*}})
// CHECK:   ret void

struct MyAttributes {
  float2 bary;
  uint id;
};

[shader("intersection")]
void intersection1()
{
  float hitT = RayTCurrent();
  MyAttributes attr = (MyAttributes)0;
  bool bReported = ReportHit(hitT, 0, attr);
}
