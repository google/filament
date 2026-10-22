// RUN: %dxc -Tlib_6_3 -Wno-unused-value -verify %s
// RUN: %dxc -Tps_6_0 -Wno-unused-value -verify %s

// we use -Wno-unused-value because we generate some no-op expressions to yield errors
// without also putting them in a static assertion

// __decltype is the GCC way of saying 'decltype', but doesn't require C++11
// _Static_assert is the C11 way of saying 'static_assert', but doesn't require C++11
#ifdef VERIFY_FXC
#define _Static_assert(a,b,c) ;
#endif

[shader("pixel")]
float4 main() : SV_Target {
  if (WaveIsFirstLane()) {
    // Divergent, single thread executing here.
  }
  uint a = WaveGetLaneIndex();

  if (WaveGetLaneCount() == 0) {
    // Unlikely!
  }
  if (WaveActiveAnyTrue(true)) {
    // Always true.
  }
  if (WaveActiveAllTrue(true)) {
    // Always true.
  }
  if (WaveActiveAllEqual(WaveGetLaneIndex())) {
    // true only if 'w' has a single lane active
  }
  uint4 val = WaveActiveBallot(true);
  if (val.x == 1) {
    // Only true if lane 0 was the only active lane at this point.
  }
  float3 f3 = { 1, 2, 3 };
  uint3 u3 = { 1, 2 ,3 };
  uint u;
  uint2 u2 = { 1, 2 };
  float f1 = 1;
  f3 = WaveReadLaneAt(f3, 1);
  f3 = WaveReadLaneFirst(f3);
  f3 = WaveActiveSum(f3);
  f3 = WaveActiveProduct(f3);
  // WaveActiveBit* with an invalid signature suggests the use of WaveActiveBallot instead.
  WaveActiveBitAnd(f1); // expected-error {{no matching function for call to 'WaveActiveBitAnd'}} expected-note {{candidate function not viable: no known conversion from 'float' to 'unsigned int' for 1st argument}}
  WaveActiveBitOr(f1);  // expected-error {{no matching function for call to 'WaveActiveBitOr'}} expected-note {{candidate function not viable: no known conversion from 'float' to 'unsigned int' for 1st argument}}
  WaveActiveBitXor(f1); // expected-error {{no matching function for call to 'WaveActiveBitXor'}} expected-note {{candidate function not viable: no known conversion from 'float' to 'unsigned int' for 1st argument}}
  u3 = WaveActiveBitAnd(u3);
  u3 = WaveActiveBitOr(u3);
  u3 = WaveActiveBitXor(u3);
  u3 = WaveActiveMin(u3);
  u3 = WaveActiveMax(u3);
  f3 = WavePrefixSum(3);
  f3 = WavePrefixProduct(3);
  f3 = QuadReadLaneAt(f3, 1);
  u = QuadReadAcrossX(u);
  u2 = QuadReadAcrossY(u2);
};
