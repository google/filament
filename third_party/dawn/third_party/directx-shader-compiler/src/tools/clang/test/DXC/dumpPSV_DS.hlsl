// REQUIRES: dxil-1-8
// RUN: %dxc -E main -T ds_6_8 %s -Fo %t
// RUN: %dxa %t -dumppsv | FileCheck %s

// CHECK:PSVRuntimeInfo:
// CHECK-NEXT:  Domain Shader
// CHECK-NEXT:  InputControlPointCount=3
// CHECK-NEXT:  OutputPositionPresent=1
// CHECK-NEXT:  MinimumExpectedWaveLaneCount: 0
// CHECK-NEXT:  MaximumExpectedWaveLaneCount: 4294967295
// CHECK-NEXT:  UsesViewID: false
// CHECK-NEXT:  SigInputElements: 4
// CHECK-NEXT:  SigOutputElements: 4
// CHECK-NEXT:  SigPatchConstOrPrimElements: 2
// CHECK-NEXT:  SigInputVectors: 4
// CHECK-NEXT:  SigOutputVectors[0]: 4
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
// CHECK-NEXT:   StartRow: 3
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 1
// CHECK-NEXT:   SemanticKind: RenderTargetArrayIndex
// CHECK-NEXT:   InterpolationMode: 1
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 1
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
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 3
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 1
// CHECK-NEXT:   SemanticKind: RenderTargetArrayIndex
// CHECK-NEXT:   InterpolationMode: 1
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 1
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
// CHECK-NEXT:   Inputs[12] influencing Outputs[0] :  None
// CHECK-NEXT:   Inputs[13] influencing Outputs[0] :  None
// CHECK-NEXT:   Inputs[14] influencing Outputs[0] :  None
// CHECK-NEXT:   Inputs[15] influencing Outputs[0] :  None
// CHECK-NEXT: Outputs affected by patch constant inputs as a table of bitmasks:
// CHECK-NEXT: PatchConstantInputs contributing to computation of Outputs:
// CHECK-NEXT:   PatchConstantInputs[0] influencing Outputs :  None
// CHECK-NEXT:   PatchConstantInputs[1] influencing Outputs :  None
// CHECK-NEXT:   PatchConstantInputs[2] influencing Outputs :  None
// CHECK-NEXT:   PatchConstantInputs[3] influencing Outputs : 4  5
// CHECK-NEXT:   PatchConstantInputs[4] influencing Outputs :  None
// CHECK-NEXT:   PatchConstantInputs[5] influencing Outputs :  None
// CHECK-NEXT:   PatchConstantInputs[6] influencing Outputs :  None
// CHECK-NEXT:   PatchConstantInputs[7] influencing Outputs : 0  1  2  3
// CHECK-NEXT:   PatchConstantInputs[8] influencing Outputs :  None
// CHECK-NEXT:   PatchConstantInputs[9] influencing Outputs :  None
// CHECK-NEXT:   PatchConstantInputs[10] influencing Outputs :  None
// CHECK-NEXT:   PatchConstantInputs[11] influencing Outputs :  None
// CHECK-NEXT:   PatchConstantInputs[12] influencing Outputs : 8  9  10
// CHECK-NEXT:   PatchConstantInputs[13] influencing Outputs :  None
// CHECK-NEXT:   PatchConstantInputs[14] influencing Outputs :  None
// CHECK-NEXT:   PatchConstantInputs[15] influencing Outputs :  None


struct PSSceneIn {
  float4 pos : SV_Position;
  float2 tex : TEXCOORD0;
  float3 norm : NORMAL;

uint   RTIndex      : SV_RenderTargetArrayIndex;
};

struct HSPerVertexData {
  // This is just the original vertex verbatim. In many real life cases this would be a
  // control point instead
  PSSceneIn v;
};

struct HSPerPatchData {
  // We at least have to specify tess factors per patch
  // As we're tesselating triangles, there will be 4 tess factors
  // In real life case this might contain face normal, for example
  float edges[3] : SV_TessFactor;
  float inside : SV_InsideTessFactor;
};

// domain shader that actually outputs the triangle vertices
[domain("tri")] PSSceneIn main(const float3 bary
                               : SV_DomainLocation,
                                 const OutputPatch<HSPerVertexData, 3> patch,
                                 const HSPerPatchData perPatchData) {
  PSSceneIn v;

  // Compute interpolated coordinates
  v.pos = patch[0].v.pos * bary.x + patch[1].v.pos * bary.y + patch[2].v.pos * bary.z + perPatchData.edges[1];
  v.tex = patch[0].v.tex * bary.x + patch[1].v.tex * bary.y + patch[2].v.tex * bary.z + perPatchData.edges[0];
  v.norm = patch[0].v.norm * bary.x + patch[1].v.norm * bary.y + patch[2].v.norm * bary.z + perPatchData.inside;
  v.RTIndex = 0;
  return v;
}
