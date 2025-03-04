// RUN: %dxc -Zi -E main -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_DB
// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s -check-prefix=CHK_NODB

// CHK_DB: 8:12: error: AddUint64 can only be applied to uint2 and uint4 operands.
// CHK_NODB: 8:12: error: AddUint64 can only be applied to uint2 and uint4 operands.

float4 main(uint4 a : A, uint4 b :B) : SV_TARGET {
  uint c = AddUint64(a.x, b.x);
  uint3 c3 = AddUint64(a.xyz, b.xyz);
  return c + c3.xyzz;
}
