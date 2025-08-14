// RUN: %dxc -T lib_6_5 -auto-binding-space 11 %s | FileCheck %s

// CHECK: define void [[intersection1:@"\\01\?intersection1@[^\"]+"]]() #0 {
// CHECK-DAG:   [[rayTCurrent:%[^ ]+]] = call float @dx.op.rayTCurrent.f32(i32 154)
// CHECK-DAG:   [[GeometryIndex:%[^ ]+]] = call i32 @dx.op.geometryIndex.i32(i32 213)
// CHECK-DAG:   icmp eq i32 [[GeometryIndex]], 0
// CHECK-DAG:   call i1 @dx.op.reportHit.struct.MyAttributes(i32 158, float [[rayTCurrent]], i32 0, %struct.MyAttributes* nonnull {{.*}})
// CHECK:   ret void

struct MyAttributes {
  float2 bary;
  uint id;
};

[shader("intersection")] void intersection1() {
  float hitT = RayTCurrent();
  MyAttributes attr = (MyAttributes)0;
  if (GeometryIndex() == 0) {
    bool bReported = ReportHit(hitT, 0, attr);  
  }
}
