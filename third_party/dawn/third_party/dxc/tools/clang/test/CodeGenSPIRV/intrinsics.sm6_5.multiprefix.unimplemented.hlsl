// RUN: not %dxc -T ps_6_5 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

StructuredBuffer<uint4> g_mask;

uint main(uint input : ATTR0) : SV_Target {
  uint4 mask = g_mask[0];

  uint res = uint4(0, 0, 0, 0);
// CHECK: error: WaveMultiPrefixCountBits intrinsic function unimplemented
  res.x += WaveMultiPrefixCountBits((input.x == 1), mask);

  return res;
}
