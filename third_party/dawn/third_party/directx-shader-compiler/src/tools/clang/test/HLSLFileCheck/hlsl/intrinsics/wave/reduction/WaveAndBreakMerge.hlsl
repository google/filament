// RUN: %dxc -T lib_6_3 %s | FileCheck %s
// Test failure expected when run with 19041 SDK DXIL.dll

// When a conditional break block follows a conditional while loop entry,
// There can be some merging of conditionals, particularly when the dx.break
// adds a conditional of its own. This ensures they are handled appropriately.

// CHECK: @dx.break.cond = internal constant

// CHECK: define i32
// CHECK-SAME: CondMergeWave
// CHECK: load i32
// CHECK-SAME: @dx.break.cond
// CHECK: icmp eq i32

// These verify the break block keeps the merged conditional
// CHECK: call i32 @dx.op.waveReadLaneFirst
// CHECK: and i1
// CHECK: br i1
// CHECK: ret i32
export
int CondMergeWave(uint Bits, float4 Bobs)
{
  while (Bits) {
    if (Bobs.a < 0.001) {
      Bits = WaveReadLaneFirst(Bits);
      break;
    }
    Bits >>= 1;
  }
  return Bits;
}

// CHECK: define i32
// CHECK-SAME: CondMerge

// Shouldn't be any use of dx.break nor any need to and anything
// CHECK-NOT: dx.break.cond
// CHECK-NOT: and i1
// CHECK: ret i32
export
int CondMerge(uint Bits, float4 Bobs)
{
  while (Bits) {
    if (Bobs.a < 0.001)
      break;
    Bits >>= 1;
  }
  return Bits;
}

