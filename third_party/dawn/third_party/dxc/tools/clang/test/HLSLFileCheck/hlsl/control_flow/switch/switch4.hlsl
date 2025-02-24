// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Ensure values through non-default cases are not eliminated
// CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59,

cbuffer CBStruct : register(b0)
{
  uint value;
};

RWStructuredBuffer<uint> Output : register(u0);

[numthreads(64, 1, 1)]
void main(uint DTid : SV_DispatchThreadID)
{
  uint copyOfValue = value;

  uint data = 0;
  switch (DTid) {
    case 0:
    case 2:
      data = copyOfValue;
      break;

    default:
      data = 0;
      break;
  }
  Output[DTid] = data;
}
