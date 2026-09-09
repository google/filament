// REQUIRES: dxil-1-9
// RUN: %dxc -T ps_6_9 %s | FileCheck %s

RWByteAddressBuffer buf;

float4 main(uint4 id: IN0) : SV_Target
{
  vector<float, 256> fVec = buf.Load<vector<float, 256> >(256 * 0);
  vector<int, 256> iVec = buf.Load<vector<int, 256> >(256 * 1);
  vector<uint, 256> uVec = buf.Load<vector<uint, 256> >(256 * 2);
  uint4 mask = {0, 1, 256, 1024};


  // Quad Broadcast

  // CHECK: call <256 x float> @dx.op.quadReadLaneAt.v256f32(i32 122, <256 x float> %{{.*}}, i32 %{{.*}})  ; QuadReadLaneAt(value,quadLane)
  fVec = QuadReadLaneAt(fVec, id.x);
  // CHECK: call <256 x float> @dx.op.quadOp.v256f32(i32 123, <256 x float> %{{.*}}, i8 0)  ; QuadOp(value,op)
  fVec = QuadReadAcrossX(fVec);
  // CHECK: call <256 x float> @dx.op.quadOp.v256f32(i32 123, <256 x float> %{{.*}}, i8 1)  ; QuadOp(value,op)
  fVec = QuadReadAcrossY(fVec);
  // CHECK: call <256 x float> @dx.op.quadOp.v256f32(i32 123, <256 x float> %{{.*}}, i8 2)  ; QuadOp(value,op)
  fVec = QuadReadAcrossDiagonal(fVec);


  // Wave Bit

  // CHECK: call <256 x i32> @dx.op.waveActiveBit.v256i32(i32 120, <256 x i32> %{{.*}}, i8 0)  ; WaveActiveBit(value,op)
  iVec += WaveActiveBitAnd(uVec);
  // CHECK: call <256 x i32> @dx.op.waveActiveBit.v256i32(i32 120, <256 x i32> %{{.*}}, i8 1)  ; WaveActiveBit(value,op)
  iVec += WaveActiveBitOr(uVec);
  // CHECK: call <256 x i32> @dx.op.waveActiveBit.v256i32(i32 120, <256 x i32> %{{.*}}, i8 2)  ; WaveActiveBit(value,op)
  iVec += WaveActiveBitXor(uVec);
  // CHECK: call <256 x i32> @dx.op.waveMultiPrefixOp.v256i32(i32 166, <256 x i32> %{{.*}}, i32 0, i32 1, i32 256, i32 1024, i8 1, i8 0)  ; WaveMultiPrefixOp(value,mask0,mask1,mask2,mask3,op,sop)
  iVec += WaveMultiPrefixBitAnd(uVec, mask);
  // CHECK: call <256 x i32> @dx.op.waveMultiPrefixOp.v256i32(i32 166, <256 x i32> %{{.*}}, i32 0, i32 1, i32 256, i32 1024, i8 2, i8 0)  ; WaveMultiPrefixOp(value,mask0,mask1,mask2,mask3,op,sop)
  iVec += WaveMultiPrefixBitOr(uVec, mask);
  // CHECK: call <256 x i32> @dx.op.waveMultiPrefixOp.v256i32(i32 166, <256 x i32> %{{.*}}, i32 0, i32 1, i32 256, i32 1024, i8 3, i8 0)  ; WaveMultiPrefixOp(value,mask0,mask1,mask2,mask3,op,sop)
  iVec += WaveMultiPrefixBitXor(uVec, mask);


  // Wave Math

  // CHECK: call <256 x float> @dx.op.waveActiveOp.v256f32(i32 119, <256 x float> %{{.*}}, i8 0, i8 0)  ; WaveActiveOp(value,op,sop)
  fVec = WaveActiveSum(fVec);
  // CHECK: call <256 x float> @dx.op.waveActiveOp.v256f32(i32 119, <256 x float> %{{.*}}, i8 1, i8 0)  ; WaveActiveOp(value,op,sop)
  fVec = WaveActiveProduct(fVec);
  // CHECK: call <256 x float> @dx.op.waveActiveOp.v256f32(i32 119, <256 x float> %{{.*}}, i8 2, i8 0)  ; WaveActiveOp(value,op,sop)
  fVec = WaveActiveMin(fVec);
  // CHECK: call <256 x float> @dx.op.waveActiveOp.v256f32(i32 119, <256 x float> %{{.*}}, i8 3, i8 0)  ; WaveActiveOp(value,op,sop)
  fVec = WaveActiveMax(fVec);
  // CHECK: call <256 x float> @dx.op.wavePrefixOp.v256f32(i32 121, <256 x float> %{{.*}}, i8 0, i8 0)  ; WavePrefixOp(value,op,sop)
  fVec = WavePrefixSum(fVec);
  // CHECK: call <256 x float> @dx.op.wavePrefixOp.v256f32(i32 121, <256 x float> %{{.*}}, i8 1, i8 0)  ; WavePrefixOp(value,op,sop)
  fVec = WavePrefixProduct(fVec);
  // CHECK: call <256 x float> @dx.op.waveMultiPrefixOp.v256f32(i32 166, <256 x float> %{{.*}}, i32 0, i32 1, i32 256, i32 1024, i8 0, i8 0)  ; WaveMultiPrefixOp(value,mask0,mask1,mask2,mask3,op,sop)
  fVec = WaveMultiPrefixSum(fVec, mask);
  // CHECK: call <256 x float> @dx.op.waveMultiPrefixOp.v256f32(i32 166, <256 x float> %{{.*}}, i32 0, i32 1, i32 256, i32 1024, i8 4, i8 0)  ; WaveMultiPrefixOp(value,mask0,mask1,mask2,mask3,op,sop)
  fVec = WaveMultiPrefixProduct(fVec, mask);


  // Wave Broadcast

  // CHECK: call <256 x float> @dx.op.waveReadLaneFirst.v256f32(i32 118, <256 x float> %{{.*}})  ; WaveReadLaneFirst(value)
  fVec = WaveReadLaneFirst(fVec);
  // CHECK: call <256 x float> @dx.op.waveReadLaneAt.v256f32(i32 117, <256 x float> %{{.*}}, i32 %{{.*}})  ; WaveReadLaneAt(value,lane)
  fVec = WaveReadLaneAt(fVec, id.x);


  // Wave Reductions

  // CHECK: call <256 x i1> @dx.op.waveActiveAllEqual.v256f32(i32 115, <256 x float> %{{.*}})  ; WaveActiveAllEqual(value)
  iVec += WaveActiveAllEqual(fVec);
  // CHECK: call %dx.types.fouri32 @dx.op.waveMatch.v256i32(i32 165, <256 x i32> %{{.*}})  ; WaveMatch(value)
  uint4 u4Vec = WaveMatch(iVec);
  iVec[0] += u4Vec.x;

  float4 ret = {
    fVec[id.x] + iVec[id.x],
    fVec[id.y] + iVec[id.y],
    fVec[id.z] + iVec[id.z],
    fVec[id.w] + iVec[id.w]
  };

  return ret;
}
