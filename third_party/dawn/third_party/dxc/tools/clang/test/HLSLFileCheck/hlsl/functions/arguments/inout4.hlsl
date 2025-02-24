// RUN: %dxc -E main -T ds_6_0 %s | FileCheck %s

// CHECK: SV_RenderTargetArrayIndex or SV_ViewportArrayIndex from any shader feeding rasterizer
// CHECK: InputControlPointCount=3
// CHECK: OutputPositionPresent=1
// CHECK: domainLocation.f32

// loadPatchConstant for the inout signature.
// CHECK: loadPatchConstant

//--------------------------------------------------------------------------------------
// SimpleTessellation.hlsl
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

struct VSSceneIn {
  float3 pos : POSITION;
  float3 norm : NORMAL;
  float2 tex : TEXCOORD0;
};

struct PSSceneIn {
  float4 pos : SV_Position;
  float2 tex : TEXCOORD0;
  float3 norm : NORMAL;

uint   RTIndex      : SV_RenderTargetArrayIndex;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Simple forwarding Tessellation shaders

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
                                 const HSPerPatchData perPatchData,
                                 inout float x : X) {
  PSSceneIn v;

  // Compute interpolated coordinates
  v.pos = patch[0].v.pos * bary.x + patch[1].v.pos * bary.y + patch[2].v.pos * bary.z;
  v.tex = patch[0].v.tex * bary.x + patch[1].v.tex * bary.y + patch[2].v.tex * bary.z;
  v.norm = patch[0].v.norm * bary.x + patch[1].v.norm * bary.y + patch[2].v.norm * bary.z;
  v.RTIndex = 0;
  return v;
}
