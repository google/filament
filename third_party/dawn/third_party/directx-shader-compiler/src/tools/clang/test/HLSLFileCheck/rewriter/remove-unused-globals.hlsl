// RUN: %dxr -E main -remove-unused-globals %s | FileCheck %s

// CHECK: cbuffer TestCBuffer
cbuffer TestCBuffer
{
// CHECK: int TestInt;
  int TestInt;
// UnusedFloat is not removed if inside a cbuffer declaration with a used global
// CHECK: float UnusedFloat;
  float UnusedFloat;
}
// CHECK: }

// Unused cbuffers are not removed at this time
// CHECK: cbuffer UnusedCBuffer
cbuffer UnusedCBuffer
{
// CHECK: float FloatInUnusedCBuffer;
  float FloatInUnusedCBuffer;
}
// CHECK: }

// CHECK-NOT: RandomGlobal
float4 RandomGlobal[2];

// CHECK: LookupTable
float4 LookupTable[1];

// CHECK-NOT: UnusedBool
bool UnusedBool;

// CHECK: TestBool
bool TestBool;

// CHECK-NOT: UnusedTex
Texture2D UnusedTex;

// CHECK: TestTex
Texture2D TestTex;

// CHECK-NOT: UnusedSampler
SamplerState UnusedSampler;

// CHECK: TestSampler
SamplerState TestSampler;

// CHECK: float3 ReferencedButDead;
float3 ReferencedButDead;

// CHECK: void main(
void main(in float4 SvPosition : SV_Position,
          out float4 OutTarget0 : SV_Target0)
{
  float3 Color = 0;
  uint Index = 0;

  if (TestBool)
  {
    Index = TestInt;
  }

  // Should keep any referenced global, even if it is unused.
  (void)ReferencedButDead;

  float2 UV = LookupTable[Index].xy + LookupTable[Index].zw;
  float4 Value = TestTex.SampleLevel(TestSampler, UV, 0);

  Color += Value.rgb + (1 - Value.a);

  OutTarget0 = float4 (Color, 1.0f);
}
