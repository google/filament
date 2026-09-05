// RUN: %dxc -auto-binding-space 13 -T lib_6_3 -export-shaders-only %s | FileCheck %s

// CHECK-NOT: unused
// CHECK-NOT: @"\01?HSPerPatchFunc1{{[@$?.A-Za-z0-9_]+}}"
// CHECK: define void @"\01?HSPerPatchFunc2
// CHECK: define void @HSMain1()
// CHECK: define void @HSMain2()
// CHECK: define void @HSMain3()
// CHECK: define void @"\01?HSPerPatchFunc1{{[@$?.A-Za-z0-9_]+}}"

Buffer<float> T_unused;

Buffer<float> GetBuffer_unused() { return T_unused; }

struct PSSceneIn
{
  float4 pos  : SV_Position;
  float2 tex  : TEXCOORD0;
  float3 norm : NORMAL;
};

struct HSPerPatchData
{
  float edges[3] : SV_TessFactor;
  float inside   : SV_InsideTessFactor;
};

struct HSPerPatchDataQuad
{
  float edges[4] : SV_TessFactor;
  float inside[2]   : SV_InsideTessFactor;
};

// Should not be selected, since later candidate function with same name exists.
// If selected, it should fail, since patch size mismatches HS function.
HSPerPatchData HSPerPatchFunc1(
  const InputPatch< PSSceneIn, 16 > points)
{
  HSPerPatchData d;

  d.edges[0] = -5;
  d.edges[1] = -6;
  d.edges[2] = -7;
  d.inside = T_unused.Load(1).x;

  return d;
}

HSPerPatchDataQuad HSPerPatchFunc2(
  const InputPatch< PSSceneIn, 4 > points)
{
  HSPerPatchDataQuad d;

  d.edges[0] = -5;
  d.edges[1] = -6;
  d.edges[2] = -7;
  d.edges[3] = -7;
  d.inside[0] = -8;
  d.inside[1] = -8;

  return d;
}


[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HSPerPatchFunc1")]
[outputcontrolpoints(3)]
void HSMain1( const uint id : SV_OutputControlPointID,
              const InputPatch< PSSceneIn, 3 > points )
{
}

[shader("hull")]
[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HSPerPatchFunc2")]
[outputcontrolpoints(4)]
void HSMain2( const uint id : SV_OutputControlPointID,
              const InputPatch< PSSceneIn, 4 > points )
{
}

[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_ccw")]
[patchconstantfunc("HSPerPatchFunc1")]
[outputcontrolpoints(3)]
void HSMain3( const uint id : SV_OutputControlPointID,
              const InputPatch< PSSceneIn, 3 > points )
{
}

// actual selected HSPerPatchFunc1 for HSMain1 and HSMain3
HSPerPatchData HSPerPatchFunc1()
{
  HSPerPatchData d;

  d.edges[0] = -5;
  d.edges[1] = -6;
  d.edges[2] = -7;
  d.inside = -8;

  return d;
}
