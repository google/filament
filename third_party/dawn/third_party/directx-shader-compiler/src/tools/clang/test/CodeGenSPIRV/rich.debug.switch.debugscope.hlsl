// RUN: %dxc -T ps_6_0 -E main -fspv-debug=rich -fcgl  %s -spirv | FileCheck %s

float4 main(float2 inUV : TEXCOORD0) : SV_TARGET
{
  int x = int(floor(inUV.x / 0.25f));
  float4 samplePos = float4(0, 0, 0, 1);
  float2 uv = float2(inUV.x * 4.0f, (inUV.y - 1.0/3.0) * 3.0);
  uv = 2.0 * float2(uv.x - float(x) * 1.0, uv.y) - 1.0;

// CHECK:      OpSelectionMerge %switch_merge None
// CHECK-NEXT: OpSwitch {{%[0-9]+}} %switch_merge

// OpBranch before a new OpLabel after OpSwitch causes a validation error.
// CHECK-NOT:  DebugScope
// CHECK-NOT:  OpBranch
// CHECK-NEXT: %switch_0 = OpLabel

  switch (x) {
    case 0: // NEGATIVE_X
      samplePos = float4(-1.0f, uv.y, uv.x, 1);
      break;
    case 1: // POSITIVE_Z
      samplePos = float4(uv.x, uv.y, 1.0f, 1);
      break;
    case 2: // POSITIVE_X
      samplePos = float4(1.0, uv.y, -uv.x, 1);
      break;
    case 3: // NEGATIVE_Z
      samplePos = float4(-uv.x, uv.y, -1.0f, 1);
      break;
  }
  return samplePos;
}
