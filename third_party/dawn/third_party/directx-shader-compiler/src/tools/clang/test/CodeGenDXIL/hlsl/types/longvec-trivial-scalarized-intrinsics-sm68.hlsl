// REQUIRES: dxil-1-9
// Tests non-native-vector behavior for intrinsics that scalarize 
//  into a simple repetition of the same dx.op calls.

// The binary part of some of these is all just a vector math ops with as many unary dxops as elements.
// These will have apparent mismatches between the ARITY define and the check prefix.

// RUN: %dxc -DFUNC=abs         -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,UNARY
// RUN: %dxc -DFUNC=pow         -DARITY=2 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,UNARY
// RUN: %dxc -DFUNC=modf        -DARITY=2 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,UNARY
// RUN: %dxc -DFUNC=ddx         -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,UNARY
// RUN: %dxc -DFUNC=ddx_coarse  -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,UNARY
// RUN: %dxc -DFUNC=ddx_fine    -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,UNARY
// RUN: %dxc -DFUNC=ddy         -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,UNARY
// RUN: %dxc -DFUNC=ddy_coarse  -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,UNARY
// RUN: %dxc -DFUNC=ddy_fine    -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,UNARY

// RUN: %dxc -DFUNC=max         -DARITY=2 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,BINARY
// RUN: %dxc -DFUNC=min         -DARITY=2 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,BINARY

// RUN: %dxc -DFUNC=countbits   -DARITY=1 -DTYPE=uint -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,UBITS
// RUN: %dxc -DFUNC=firstbithigh -DARITY=1 -DTYPE=uint -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,UBITS
// RUN: %dxc -DFUNC=firstbitlow  -DARITY=1 -DTYPE=uint -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,UBITS

// RUN: %dxc -DFUNC=isfinite    -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,SPECFLT
// RUN: %dxc -DFUNC=isinf       -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,SPECFLT
// RUN: %dxc -DFUNC=isnan       -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,SPECFLT
// RUN: %dxc -DFUNC=isnormal    -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,SPECFLT

// RUN: %dxc -DFUNC=QuadReadLaneAt         -DARITY=4 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,QUAD
// RUN: %dxc -DFUNC=QuadReadAcrossX        -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,QUAD
// RUN: %dxc -DFUNC=QuadReadAcrossY        -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,QUAD
// RUN: %dxc -DFUNC=QuadReadAcrossDiagonal -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,QUAD
// RUN: %dxc -DFUNC=WaveActiveBitAnd       -DARITY=1 -DTYPE=uint -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveActiveBitOr        -DARITY=1 -DTYPE=uint -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveActiveBitXor       -DARITY=1 -DTYPE=uint -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveActiveProduct      -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveActiveSum          -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveActiveMin          -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveActiveMax          -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveMultiPrefixBitAnd  -DARITY=5 -DTYPE=uint -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveMultiPrefixBitOr   -DARITY=5 -DTYPE=uint -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveMultiPrefixBitXor  -DARITY=5 -DTYPE=uint -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveMultiPrefixProduct -DARITY=5 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveMultiPrefixSum     -DARITY=5 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WavePrefixSum          -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WavePrefixProduct      -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveReadLaneAt         -DARITY=4 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveReadLaneFirst      -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE
// RUN: %dxc -DFUNC=WaveActiveAllEqual     -DARITY=1 -T ps_6_8 %s | FileCheck %s --check-prefixes=CHECK,WAVE

// Test a smaller subset compiling with 6.9 and linking with 6.8 targets.

// RUN: %dxc -DFUNC=pow        -DARITY=2 -T lib_6_9 %s -Fo %t.1
// RUN: %dxl -T ps_6_8 %t.1 | FileCheck %s --check-prefixes=CHECK,UNARY
// RUN: %dxc -DFUNC=countbits  -DARITY=1 -DTYPE=uint -T lib_6_9 %s -Fo %t.1
// RUN: %dxl -T ps_6_8 %t.1 | FileCheck %s --check-prefixes=CHECK,UNARY
// RUN: %dxc -DFUNC=ddx         -DARITY=1 -T lib_6_9 %s -Fo %t.4
// RUN: %dxl -T ps_6_8 %t.4 | FileCheck %s --check-prefixes=CHECK,UNARY
// RUN: %dxc -DFUNC=isinf       -DARITY=1 -T ps_6_8 %s -T lib_6_9 -Fo %t.1
// RUN: %dxl -T ps_6_8 %t.1 | FileCheck %s --check-prefixes=CHECK,SPECFLT
// RUN: %dxc -DFUNC=QuadReadLaneAt -DARITY=4 -T lib_6_9 %s -Fo %t.1
// RUN: %dxl -T ps_6_8 %t.1 | FileCheck %s --check-prefixes=CHECK,QUAD
// RUN: %dxc -DFUNC=WaveActiveMax  -DARITY=1 -T lib_6_9 %s -Fo %t.1
// RUN: %dxl -T ps_6_8 %t.1 | FileCheck %s --check-prefixes=CHECK,WAVE

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

ByteAddressBuffer rbuf;

// CHECK-LABEL: define void @main()
[shader("pixel")]
float4 main(uint i : SV_PrimitiveID, uint4 m : M) : SV_Target {
  vector<TYPE, 4> arg1 = rbuf.Load< vector<TYPE, 4> >(i++*32);
  vector<TYPE, 4> arg2 = rbuf.Load< vector<TYPE, 4> >(i++*32);
  vector<TYPE, 4> arg3 = rbuf.Load< vector<TYPE, 4> >(i++*32);

  // UNARY: call {{.*}} [[DXOP:@dx.op.unary.]]
  // UBITS: call {{.*}} [[DXOP:@dx.op.unaryBits.]]
  // BINARY: call {{.*}} [[DXOP:@dx.op.binary.]]
  // SPECFLT: call {{.*}} [[DXOP:@dx.op.isSpecialFloat.]]
  // QUAD: call {{.*}} [[DXOP:@dx.op.quad.]]
  // WAVE: call {{.*}} [[DXOP:@dx.op.wave.]]
  // CHECK: call {{.*}} [[DXOP]]
  // CHECK: call {{.*}} [[DXOP]]
  // CHECK: call {{.*}} [[DXOP]]

  return FUNC(CALLARGS(arg1, arg2, arg3));
}
