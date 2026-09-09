// REQUIRES: dxil-1-8
// RUN: %dxc -E main -T ps_6_8 %s -Fo %t
// RUN: %dxa %t -dumppsv | FileCheck %s

// CHECK:DxilPipelineStateValidation:
// CHECK-NEXT: PSVRuntimeInfo:
// CHECK-NEXT:  Pixel Shader
// CHECK-NEXT:  DepthOutput=0
// CHECK-NEXT:  SampleFrequency=1
// CHECK-NEXT:  MinimumExpectedWaveLaneCount: 0
// CHECK-NEXT:  MaximumExpectedWaveLaneCount: 4294967295
// CHECK-NEXT:  UsesViewID: false
// CHECK-NEXT:  SigInputElements: 2
// CHECK-NEXT:  SigOutputElements: 1
// CHECK-NEXT:  SigPatchConstOrPrimElements: 0
// CHECK-NEXT:  SigInputVectors: 2
// CHECK-NEXT:  SigOutputVectors[0]: 1
// CHECK-NEXT:  SigOutputVectors[1]: 0
// CHECK-NEXT:  SigOutputVectors[2]: 0
// CHECK-NEXT:  SigOutputVectors[3]: 0
// CHECK-NEXT:  EntryFunctionName: main
// CHECK-NEXT: ResourceCount : 3
// CHECK-NEXT:  PSVResourceBindInfo:
// CHECK-NEXT:   Space: 0
// CHECK-NEXT:   LowerBound: 1
// CHECK-NEXT:   UpperBound: 1
// CHECK-NEXT:   ResType: CBV
// CHECK-NEXT:   ResKind: CBuffer
// CHECK-NEXT:   ResFlags: None
// CHECK-NEXT: PSVResourceBindInfo:
// CHECK-NEXT:   Space: 0
// CHECK-NEXT:   LowerBound: 0
// CHECK-NEXT:   UpperBound: 0
// CHECK-NEXT:   ResType: Sampler
// CHECK-NEXT:   ResKind: Sampler
// CHECK-NEXT:   ResFlags: None
// CHECK-NEXT: PSVResourceBindInfo:
// CHECK-NEXT:   Space: 0
// CHECK-NEXT:   LowerBound: 0
// CHECK-NEXT:   UpperBound: 0
// CHECK-NEXT:   ResType: SRVTyped
// CHECK-NEXT:   ResKind: Texture2D
// CHECK-NEXT:   ResFlags: None
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: NORMAL
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 0
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 3
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 6
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: TEXCOORD
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 1
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 2
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 4
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName:
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 0
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 4
// CHECK-NEXT:   SemanticKind: Target
// CHECK-NEXT:   InterpolationMode: 0
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: Outputs affected by inputs as a table of bitmasks for stream 0:
// CHECK-NEXT: Inputs contributing to computation of Outputs[0]:
// CHECK-NEXT:   Inputs[0] influencing Outputs[0] : 0  1  2  3
// CHECK-NEXT:   Inputs[1] influencing Outputs[0] : 0  1  2  3
// CHECK-NEXT:   Inputs[2] influencing Outputs[0] : 0  1  2  3
// CHECK-NEXT:   Inputs[3] influencing Outputs[0] :  None
// CHECK-NEXT:   Inputs[4] influencing Outputs[0] : 0  1  2  3
// CHECK-NEXT:   Inputs[5] influencing Outputs[0] : 0  1  2  3
// CHECK-NEXT:   Inputs[6] influencing Outputs[0] :  None
// CHECK-NEXT:   Inputs[7] influencing Outputs[0] :  None

cbuffer cbPerObject : register( b0 )
{
    float4    g_vObjectColor    : packoffset( c0 );
};

cbuffer cbPerFrame : register( b1 )
{
    float3    g_vLightDir    : packoffset( c0 );
    float    g_fAmbient    : packoffset( c0.w );
};

Texture2D    g_txDiffuse : register( t0 );
SamplerState    g_samLinear : register( s0 );

struct PS_INPUT
{
  sample          float3 vNormal    : NORMAL;
  noperspective   float2 vTexcoord  : TEXCOORD0;
};

float4 main( PS_INPUT Input) : SV_TARGET
{
    float4 vDiffuse = g_txDiffuse.Sample( g_samLinear, Input.vTexcoord );
    
    float fLighting = saturate( dot( g_vLightDir, Input.vNormal ) );
    fLighting = max( fLighting, g_fAmbient );
    
    return vDiffuse * fLighting;
}
