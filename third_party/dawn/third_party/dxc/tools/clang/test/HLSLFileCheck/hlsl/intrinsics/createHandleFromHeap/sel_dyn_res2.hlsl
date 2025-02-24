// RUN: %dxc -T ps_6_6 %s | %FileCheck %s -check-prefixes=CHECK,CHECKO3
// RUN: %dxc -T ps_6_6 -Od %s | %FileCheck %s -check-prefixes=CHECK,CHECKOD

// CHECK-NOT:phi %dx.types.Handle
// CHECK:%[[IDX0:.*]] = phi i32 [ 0, %{{.*}} ], [ 1, %{{.*}} ]
// CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 %[[IDX0]], i1 false, i1 false
// CHECK:%[[IDX1:.*]] = phi i32 [ 2, %{{.*}} ], [ 3, %{{.*}} ]
// CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 %[[IDX1]], i1 false, i1 false
// CHECKO3:%[[IDX_SEL:.*]] = select i1 %{{.*}}, i32 %[[IDX0]], i32 %[[IDX1]]
// CHECKOD:%[[IDX_SEL:.*]] = phi i32 [ %[[IDX0]], %{{.*}} ], [ %[[IDX1]], %{{.*}} ]
// CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 %[[IDX_SEL]], i1 false, i1 false

int c;
float main() : SV_Target {
  ByteAddressBuffer b = c>0?ResourceDescriptorHeap[0]:ResourceDescriptorHeap[1];
  float r = b.Load(0);

  ByteAddressBuffer b2 = c>3?ResourceDescriptorHeap[2]:ResourceDescriptorHeap[3];
  r += b2.Load(0);
  ByteAddressBuffer b3 = c > 10 ? b : b2;
  r *= b3.Load(0);
  return r;
}