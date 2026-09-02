// REQUIRES: dxil-1-8
// RUN: %dxc -E main -T gs_6_8 %s -Fo %t
// RUN: %dxa %t -dumppsv | FileCheck %s

// CHECK:DxilPipelineStateValidation:
// CHECK-NEXT: PSVRuntimeInfo:
// CHECK-NEXT:  Geometry Shader
// CHECK-NEXT:  InputPrimitive=point
// CHECK-NEXT:  OutputTopology=triangle
// CHECK-NEXT:  OutputStreamMask=1
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
// CHECK-NEXT:   LowerBound: 0
// CHECK-NEXT:   UpperBound: 0
// CHECK-NEXT:   ResType: CBV
// CHECK-NEXT:   ResKind: CBuffer
// CHECK-NEXT:   ResFlags: None
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: POSSIZE
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
// CHECK-NEXT:   SemanticName: COLOR
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 1
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 4
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
// CHECK-NEXT:   Cols: 1
// CHECK-NEXT:   SemanticKind: RenderTargetArrayIndex
// CHECK-NEXT:   InterpolationMode: 1
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 1
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: TEXCOORD
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 0
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 2
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 2
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: COLOR
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 1
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 4
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
// CHECK-NEXT:   Inputs[0] influencing Outputs[0] : 8
// CHECK-NEXT:   Inputs[1] influencing Outputs[0] : 9
// CHECK-NEXT:   Inputs[2] influencing Outputs[0] : 8  9
// CHECK-NEXT:   Inputs[3] influencing Outputs[0] :  None
// CHECK-NEXT:   Inputs[4] influencing Outputs[0] : 4
// CHECK-NEXT:   Inputs[5] influencing Outputs[0] : 5
// CHECK-NEXT:   Inputs[6] influencing Outputs[0] : 6
// CHECK-NEXT:   Inputs[7] influencing Outputs[0] : 7
// CHECK-NEXT:   Inputs[8] influencing Outputs[0] :  None
// CHECK-NEXT:   Inputs[9] influencing Outputs[0] :  None
// CHECK-NEXT:   Inputs[10] influencing Outputs[0] :  None
// CHECK-NEXT:   Inputs[11] influencing Outputs[0] :  None
// CHECK-NEXT: Outputs affected by inputs as a table of bitmasks for stream 1:
// CHECK-NEXT: Inputs contributing to computation of Outputs[1]:  None
// CHECK-NEXT: Outputs affected by inputs as a table of bitmasks for stream 2:
// CHECK-NEXT: Inputs contributing to computation of Outputs[2]:  None
// CHECK-NEXT: Outputs affected by inputs as a table of bitmasks for stream 3:
// CHECK-NEXT: Inputs contributing to computation of Outputs[3]:  None

struct VSOut {
  float2 uv : TEXCOORD0;
  float4 clr : COLOR;
  float4 pos : SV_Position;
};

struct VSOutGSIn {
  float3 posSize : POSSIZE;
  float4 clr : COLOR;
  uint index :SV_RenderTargetArrayIndex;
};

struct VSOutGSArrayIn {
  float3 posSize : POSSIZE;
  float2 clr[2] : COLOR;
};

struct VSOutGSMatIn {
  float3 posSize : POSSIZE;
  float2x2 clr[2] : COLOR;
};

cbuffer b : register(b0) {
  float2 invViewportSize;
};

float4 NDC(float2 screen) {
  screen *= invViewportSize * 2;
  screen.x = screen.x - 1;
  screen.y = 1 - screen.y;

  return float4(screen, 0.5f, 1);
}

// geometry shader that outputs 3 vertices from a point
[maxvertexcount(3)] void main(point VSOutGSIn points[1], inout TriangleStream<VSOut> stream) {
  VSOut v;

  const float2 verts[3] =
      {
          float2(-0.5f, -0.5f),
          float2(1.5f, -0.5f),
          float2(-0.5f, 1.5f)};

  const float sz = points[0].posSize.z;
  const float2 org = points[0].posSize.xy;
  const float4 clr = float4(points[0].clr); //[0][1], points[ 0 ].clr[1][0]);

  // triangle strip for the particle

  v.uv = float2(0, 0);
  v.clr = clr;
  v.pos = NDC(org + verts[0] * sz);

  stream.Append(v);

  v.uv = float2(2, 0);
  v.clr = clr;
  v.pos = NDC(org + verts[1] * sz);

  stream.Append(v);

  v.uv = float2(0, 2);
  v.clr = clr;
  v.pos = NDC(org + verts[2] * sz);

  stream.Append(v);
  stream.RestartStrip();
}
