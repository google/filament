// REQUIRES: dxil-1-8
// RUN: %dxc -E main -T vs_6_8 %s -Fo %t
// RUN: %dxa %t -dumppsv | FileCheck %s

// CHECK:DxilPipelineStateValidation:
// CHECK-NEXT: PSVRuntimeInfo:
// CHECK-NEXT:  Vertex Shader
// CHECK-NEXT:  OutputPositionPresent=1
// CHECK-NEXT:  MinimumExpectedWaveLaneCount: 0
// CHECK-NEXT:  MaximumExpectedWaveLaneCount: 4294967295
// CHECK-NEXT:  UsesViewID: false
// CHECK-NEXT:  SigInputElements: 3
// CHECK-NEXT:  SigOutputElements: 3
// CHECK-NEXT:  SigPatchConstOrPrimElements: 0
// CHECK-NEXT:  SigInputVectors: 3
// CHECK-NEXT:  SigOutputVectors[0]: 3
// CHECK-NEXT:  SigOutputVectors[1]: 0
// CHECK-NEXT:  SigOutputVectors[2]: 0
// CHECK-NEXT:  SigOutputVectors[3]: 0
// CHECK-NEXT:  EntryFunctionName: main
// CHECK-NEXT: ResourceCount : 1
// CHECK-NEXT:  PSVResourceBindInfo:
// CHECK-NEXT:   Space: 0
// CHECK-NEXT:   LowerBound: 5
// CHECK-NEXT:   UpperBound: 5
// CHECK-NEXT:   ResType: CBV
// CHECK-NEXT:   ResKind: CBuffer
// CHECK-NEXT:   ResFlags: None
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: POSITION
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 0
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 3
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 0
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: NORMAL
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 1
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 3
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 0
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: TEXCOORD
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 2
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 2
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 0
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: NORMAL
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 0
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 3
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 2
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
// CHECK-NEXT:   InterpolationMode: 2
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName:
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 2
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 4
// CHECK-NEXT:   SemanticKind: Position
// CHECK-NEXT:   InterpolationMode: 4
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: Outputs affected by inputs as a table of bitmasks for stream 0:
// CHECK-NEXT: Inputs contributing to computation of Outputs[0]:
// CHECK-NEXT:   Inputs[0] influencing Outputs[0] : 8  9  10  11
// CHECK-NEXT:   Inputs[1] influencing Outputs[0] : 8  9  10  11
// CHECK-NEXT:   Inputs[2] influencing Outputs[0] : 8  9  10  11
// CHECK-NEXT:   Inputs[3] influencing Outputs[0] :  None
// CHECK-NEXT:   Inputs[4] influencing Outputs[0] : 0  1  2
// CHECK-NEXT:   Inputs[5] influencing Outputs[0] : 0  1  2
// CHECK-NEXT:   Inputs[6] influencing Outputs[0] : 0  1  2
// CHECK-NEXT:   Inputs[7] influencing Outputs[0] :  None
// CHECK-NEXT:   Inputs[8] influencing Outputs[0] : 4
// CHECK-NEXT:   Inputs[9] influencing Outputs[0] : 5
// CHECK-NEXT:   Inputs[10] influencing Outputs[0] :  None
// CHECK-NEXT:   Inputs[11] influencing Outputs[0] :  None

cbuffer cbPerObject : register( b5 )
{
	matrix		g_mWorldViewProjection	: packoffset( c0 );
	column_major  matrix		g_mWorld		: packoffset( c4 );
};

struct VS_INPUT
{
	float3 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
 	float2 vTexcoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
	float4 vPosition	: SV_POSITION;
};



VS_OUTPUT main( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vPosition = mul( float4( Input.vPosition, 1.0 ), g_mWorldViewProjection );
	Output.vNormal = mul( Input.vNormal, (float3x3)g_mWorld );
	Output.vTexcoord = Input.vTexcoord;
	
	return Output;
}
