// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Wave level operations
// CHECK: call i1 @dx.op.waveIsFirstLane(i32 110
// CHECK: call i32 @dx.op.waveGetLaneIndex(i32 111
// CHECK: call i32 @dx.op.waveGetLaneCount(i32 112
// CHECK: call i1 @dx.op.waveAnyTrue(i32 113, i1
// CHECK: call i1 @dx.op.waveAllTrue(i32 114, i1
// CHECK: call i1 @dx.op.waveActiveAllEqual.i32(i32 115, i32
// CHECK: call i1 @dx.op.waveActiveAllEqual.i1(i32 115, i1
// CHECK: call %dx.types.fouri32 @dx.op.waveActiveBallot(i32 116, i1
// CHECK: call float @dx.op.waveReadLaneAt.f32(i32 117, float
// CHECK: call float @dx.op.waveReadLaneFirst.f32(i32 118, float
// CHECK: call float @dx.op.waveActiveOp.f32(i32 119, float
// CHECK: call float @dx.op.waveActiveOp.f32(i32 119, float
// CHECK: call i32 @dx.op.waveActiveBit.i32(i32 120, i32
// CHECK: call i32 @dx.op.waveActiveBit.i32(i32 120, i32
// CHECK: call i32 @dx.op.waveActiveBit.i32(i32 120, i32
// CHECK: call i32 @dx.op.waveActiveOp.i32(i32 119, i32
// CHECK: call i32 @dx.op.waveActiveOp.i32(i32 119, i32
// CHECK: call float @dx.op.quadReadLaneAt.f32(i32 122, float
// CHECK: call i32 @dx.op.quadOp.i32(i32 123, i32
// CHECK: call i32 @dx.op.quadOp.i32(i32 123, i32

float4 main() : SV_TARGET {
  float f = 1;
  if (WaveIsFirstLane()) {
    f += 1;
  }
  f += WaveGetLaneIndex();
  if (WaveGetLaneCount() == 0) {
    f += 1;
  }
  if (WaveActiveAnyTrue(true)) {
    f += 1;
  }
  if (WaveActiveAllTrue(true)) {
    f += 1;
  }
  if (WaveActiveAllEqual(WaveGetLaneIndex())) {
    f += 1;
  }
  if(WaveActiveAllEqual((f<100))){
    f += 1;
  }
  uint4 val = WaveActiveBallot(true);
  if (val.x == 1) {
    f += 1;
  }
  float3 f3 = { 1, 2, 3 };
  uint3 u3 = { 1, 2 ,3 };
  uint u = 0;
  uint2 u2 = { 1, 2 };
  int i_signed = -2;
  f += WaveReadLaneAt(f3, 1).x;
  f3 += WaveReadLaneFirst(f3).x;
  f3 += WaveActiveSum(f3).x;
  f3 += WaveActiveProduct(f3).x;
  u3 += WaveActiveBitAnd(u3);
  u3 += WaveActiveBitOr(u3);
  u3 += WaveActiveBitXor(u3);
  u3 += WaveActiveMin(u3);
  u3 += WaveActiveMax(u3);
  u3 += abs(WaveActiveMin(i_signed));
  u3 += abs(WaveActiveMax(i_signed));
  f3 += WavePrefixSum(3);
  f3 += WavePrefixProduct(3);
  f3 = QuadReadLaneAt(f3, 1);
  u += QuadReadAcrossX(u);
  u2 = QuadReadAcrossY(u2);
  u2 = QuadReadAcrossDiagonal(u2);
  u += WaveActiveCountBits(1);
  u += WavePrefixCountBits(1);
  return f + f3.x + u + u2.x + u3.y;
}
