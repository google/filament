// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Test that conversions between memory/register representation happens
// when loading/storing bools from/to byte address buffers

RWByteAddressBuffer buf;

void main()
{
  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32
  // CHECK: icmp ne i32 %{{.*}}, 0
  // CHECK: zext i1 %{{.*}} to i32
  // CHECK: call void @dx.op.rawBufferStore.i32
  buf.Store(0, buf.Load<bool>(32));

  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32
  // CHECK: icmp ne i32 %{{.*}}, 0
  // CHECK: icmp ne i32 %{{.*}}, 0
  // CHECK: zext i1 %{{.*}} to i32
  // CHECK: zext i1 %{{.*}} to i32
  // CHECK: call void @dx.op.rawBufferStore.i32
  buf.Store(100, buf.Load<bool2>(132));

  // CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32
  // CHECK: icmp ne i32 %{{.*}}, 0
  // CHECK: icmp ne i32 %{{.*}}, 0
  // CHECK: icmp ne i32 %{{.*}}, 0
  // CHECK: icmp ne i32 %{{.*}}, 0
  // CHECK: zext i1 %{{.*}} to i32
  // CHECK: zext i1 %{{.*}} to i32
  // CHECK: zext i1 %{{.*}} to i32
  // CHECK: zext i1 %{{.*}} to i32
  // CHECK: call void @dx.op.rawBufferStore.i32
  buf.Store(200, buf.Load<bool2x2>(232));
}