// RUN: %dxc -E main -T ps_6_0 %s | FileCheck -input-file=stderr %s

// CHECK: 37:7: warning: Gradient operations are not affected by wave-sensitive data or control flow.
// CHECK: 30:16: warning: Gradient operations are not affected by wave-sensitive data or control flow.
// CHECK: 9:16: warning: Gradient operations are not affected by wave-sensitive data or control flow.

float4 main(float4 p: SV_Position) : SV_Target {
  // cannot feed into ddx
  float4 sum = WavePrefixSum(p);
  sum = ddx(sum);

  WaveReadLaneAt(p.x, 30); // should warn: don't make assumptions about lanes

  float x = p.x;
  uint distinct = 0;
  bool done = false;
  while (!done) {
    float m = WaveActiveMax(x);
    if (m == 0) {
      done = true;
    }
    else {
      x = 0;
      distinct++;
    }
  }
  // distinct in ddx would force implementations to run on helper lanes, but flow
  // loop would be disturbed; in the case above, helpers would not exit.
  // Any value that feeds into ddx that is helper-lane-sensitive is disallowed.
  sum.x += ddx(distinct);

  // Even without a loop, this will fail. This allows an implementation to
  // disable helper lanes altogether to avoid contributions; mn need never
  // be calculated.
  float uni = 1;
  float wavemin = WaveActiveMin(x);
  if (wavemin > 0) {
    uni = 2;
  }
  sum.x += ddx(uni);

  return sum;
}
