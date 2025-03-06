// REQUIRES: dxil-1-8
// RUN: %dxc -E main -T hs_6_8 %s -Fo %t
// RUN: %dxa %t -dumppsv | FileCheck %s

// CHECK:DxilPipelineStateValidation:
// CHECK-NEXT: PSVRuntimeInfo:
// CHECK-NEXT:  Hull Shader
// CHECK-NEXT:  InputControlPointCount=3
// CHECK-NEXT:  OutputControlPointCount=3
// CHECK-NEXT:  Domain=tri
// CHECK-NEXT:  OutputPrimitive=triangle_cw
// CHECK-NEXT:  MinimumExpectedWaveLaneCount: 0
// CHECK-NEXT:  MaximumExpectedWaveLaneCount: 4294967295
// CHECK-NEXT:  UsesViewID: true
// CHECK-NEXT:  SigInputElements: 3
// CHECK-NEXT:  SigOutputElements: 3
// CHECK-NEXT:  SigPatchConstOrPrimElements: 2
// CHECK-NEXT:  SigInputVectors: 3
// CHECK-NEXT:  SigOutputVectors[0]: 3
// CHECK-NEXT:  SigOutputVectors[1]: 0
// CHECK-NEXT:  SigOutputVectors[2]: 0
// CHECK-NEXT:  SigOutputVectors[3]: 0
// CHECK-NEXT:  EntryFunctionName: main
// CHECK-NEXT: ResourceCount : 0
// CHECK-NEXT:  PSVSignatureElement:
// CHECK-NEXT:   SemanticName:
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 0
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 4
// CHECK-NEXT:   SemanticKind: Position
// CHECK-NEXT:   InterpolationMode: 4
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
// CHECK-NEXT:   SemanticName: NORMAL
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 2
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 3
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 2
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
// CHECK-NEXT:   SemanticKind: Position
// CHECK-NEXT:   InterpolationMode: 4
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
// CHECK-NEXT:   SemanticName: NORMAL
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 2
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 3
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 2
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName:
// CHECK-NEXT:   SemanticIndex: 0 1 2
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 0
// CHECK-NEXT:   StartCol: 3
// CHECK-NEXT:   Rows: 3
// CHECK-NEXT:   Cols: 1
// CHECK-NEXT:   SemanticKind: TessFactor
// CHECK-NEXT:   InterpolationMode: 0
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName:
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 3
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 1
// CHECK-NEXT:   SemanticKind: InsideTessFactor
// CHECK-NEXT:   InterpolationMode: 0
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: Outputs affected by ViewID as a bitmask for stream 0:
// CHECK-NEXT:    ViewID influencing Outputs[0] : 8  9  10
// CHECK-NEXT: PCOutputs affected by ViewID as a bitmask:
// CHECK-NEXT:   ViewID influencing PCOutputs : 12
// CHECK-NEXT: Outputs affected by inputs as a table of bitmasks for stream 0:
// CHECK-NEXT: Inputs contributing to computation of Outputs[0]:
// CHECK-NEXT:   Inputs[0] influencing Outputs[0] : 0
// CHECK-NEXT:   Inputs[1] influencing Outputs[0] : 1
// CHECK-NEXT:   Inputs[2] influencing Outputs[0] : 2
// CHECK-NEXT:   Inputs[3] influencing Outputs[0] : 3
// CHECK-NEXT:   Inputs[4] influencing Outputs[0] : 4
// CHECK-NEXT:   Inputs[5] influencing Outputs[0] : 5
// CHECK-NEXT:   Inputs[6] influencing Outputs[0] :  None
// CHECK-NEXT:   Inputs[7] influencing Outputs[0] :  None
// CHECK-NEXT:   Inputs[8] influencing Outputs[0] : 8
// CHECK-NEXT:   Inputs[9] influencing Outputs[0] : 9
// CHECK-NEXT:   Inputs[10] influencing Outputs[0] : 10
// CHECK-NEXT:   Inputs[11] influencing Outputs[0] :  None
// CHECK-NEXT: Patch constant outputs affected by inputs as a table of bitmasks:
// CHECK-NEXT: Inputs contributing to computation of PatchConstantOutputs:
// CHECK-NEXT:   Inputs[0] influencing PatchConstantOutputs :  3
// CHECK-NEXT:   Inputs[1] influencing PatchConstantOutputs :  None
// CHECK-NEXT:   Inputs[2] influencing PatchConstantOutputs :  None
// CHECK-NEXT:   Inputs[3] influencing PatchConstantOutputs :  None
// CHECK-NEXT:   Inputs[4] influencing PatchConstantOutputs :  None
// CHECK-NEXT:   Inputs[5] influencing PatchConstantOutputs :  None
// CHECK-NEXT:   Inputs[6] influencing PatchConstantOutputs :  None
// CHECK-NEXT:   Inputs[7] influencing PatchConstantOutputs :  None
// CHECK-NEXT:   Inputs[8] influencing PatchConstantOutputs :  None
// CHECK-NEXT:   Inputs[9] influencing PatchConstantOutputs :  None
// CHECK-NEXT:   Inputs[10] influencing PatchConstantOutputs :  None
// CHECK-NEXT:   Inputs[11] influencing PatchConstantOutputs :  None

struct PSSceneIn
{
    float4 pos  : SV_Position;
    float2 tex  : TEXCOORD0;
    float3 norm : NORMAL;
};


struct HSPerVertexData
{
    // This is just the original vertex verbatim. In many real life cases this would be a
    // control point instead
    PSSceneIn v;
};

struct HSPerPatchData
{
    // We at least have to specify tess factors per patch
    // As we're tesselating triangles, there will be 4 tess factors
    // In real life case this might contain face normal, for example
	float	edges[ 3 ]	: SV_TessFactor;
	float	inside		: SV_InsideTessFactor;
};

float4 HSPerPatchFunc()
{
    return 1.8;
}

HSPerPatchData HSPerPatchFunc( const InputPatch< PSSceneIn, 3 > points ,uint vid : SV_ViewID)
{
    HSPerPatchData d;

    d.edges[ 0 ] = points[0].pos.x;
    d.edges[ 1 ] = 1;
    d.edges[ 2 ] = 1;
    d.inside = vid;

    return d;
}


[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HSPerPatchFunc")]
[outputcontrolpoints(3)]
HSPerVertexData main( const uint id : SV_OutputControlPointID,
                      uint vid : SV_ViewID,
                      const InputPatch< PSSceneIn, 3 > points )
{
    HSPerVertexData v;

    // Just forward the vertex
    v.v = points[ id ];
    v.v.norm += vid;
	return v;
}
