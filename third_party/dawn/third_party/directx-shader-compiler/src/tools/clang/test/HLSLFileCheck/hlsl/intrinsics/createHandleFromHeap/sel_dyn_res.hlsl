// RUN: %dxc -T ps_6_6 %s | %FileCheck %s
// RUN: %dxc -T ps_6_6 -Od %s | %FileCheck %s

// CHECK-NOT:phi %dx.types.Handle
// CHECK:%[[IDX:.*]] = phi i32 [ 0, %{{.*}} ], [ 1, %{{.*}} ]
// CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 %[[IDX]], i1 false, i1 false
int c;
float main() : SV_Target {
  ByteAddressBuffer b = c>0?ResourceDescriptorHeap[0]:ResourceDescriptorHeap[1];
  return b.Load(0);
}