// The binary part of some of these is all just a vector math ops with as many unary dxops as elements.
// These will have apparent mismatches between the ARITY define and the check prefix.

// RUN: %dxc -DFUNC=QuadReadLaneAt         -DARITY=4 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,QUAD
// RUN: %dxc -DFUNC=QuadReadAcrossX        -DARITY=1 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,QUAD
// RUN: %dxc -DFUNC=QuadReadAcrossY        -DARITY=1 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,QUAD
// RUN: %dxc -DFUNC=QuadReadAcrossDiagonal -DARITY=1 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,QUAD
// RUN: %dxc -DFUNC=WaveActiveBitAnd       -DARITY=1 -DTYPE=uint -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveActiveBitOr        -DARITY=1 -DTYPE=uint -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveActiveBitXor       -DARITY=1 -DTYPE=uint -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveActiveProduct      -DARITY=1 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveActiveSum          -DARITY=1 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveActiveMin          -DARITY=1 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveActiveMax          -DARITY=1 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveMultiPrefixBitAnd  -DARITY=5 -DTYPE=uint -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveMultiPrefixBitOr   -DARITY=5 -DTYPE=uint -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveMultiPrefixBitXor  -DARITY=5 -DTYPE=uint -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveMultiPrefixProduct -DARITY=5 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveMultiPrefixSum     -DARITY=5 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WavePrefixSum          -DARITY=1 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WavePrefixProduct      -DARITY=1 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveReadLaneAt         -DARITY=4 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveReadLaneFirst      -DARITY=1 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveActiveAllEqual     -DARITY=1 -T ps_6_9 %s | FileCheck %s --check-prefixes=CHECK,WAVE

#ifndef TYPE
#define TYPE float
#endif

#if ARITY == 1
#define CALLARGS(x,y,z) x
#elif ARITY == 2
#define CALLARGS(x,y,z) x, y
#elif ARITY == 3
#define CALLARGS(x,y,z) x, y, z
// ARITY 4 is used for 1 vec + scalar
#elif ARITY == 4
#define CALLARGS(x,y,z) x, i
// ARITY 5 is used for 1 vec + uint4 mask for wavemultiprefix*
#elif ARITY == 5
#define CALLARGS(x,y,z) x, m
#endif

StructuredBuffer< vector<TYPE, 8> > buf;
ByteAddressBuffer rbuf;

float4 main(uint i : SV_PrimitiveID, uint4 m : M) : SV_Target {
  vector<TYPE, 8> arg1 = rbuf.Load< vector<TYPE, 8> >(i++*32);
  vector<TYPE, 8> arg2 = rbuf.Load< vector<TYPE, 8> >(i++*32);
  vector<TYPE, 8> arg3 = rbuf.Load< vector<TYPE, 8> >(i++*32);

  // UNARY: call {{.*}} [[DXOP:@dx.op.unary]]
  // QUAD: call {{.*}} [[DXOP:@dx.op.quad]]
  // WAVE: call {{.*}} [[DXOP:@dx.op.wave]]
  // CHECK: call {{.*}} [[DXOP]]
  // CHECK: call {{.*}} [[DXOP]]
  // CHECK: call {{.*}} [[DXOP]]
  // CHECK: call {{.*}} [[DXOP]]
  // CHECK: call {{.*}} [[DXOP]]
  // CHECK: call {{.*}} [[DXOP]]
  // CHECK: call {{.*}} [[DXOP]]

  vector<TYPE, 8> ret = FUNC(CALLARGS(arg1, arg2, arg3));
  return float4(ret[0] + ret[1], ret[2] + ret[3], ret[4] + ret[5], ret[6] + ret[7]);
}
