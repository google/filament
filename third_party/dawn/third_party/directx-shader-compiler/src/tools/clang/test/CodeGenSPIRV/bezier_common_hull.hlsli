#define MAX_POINTS 16

// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
  float3 vPosition : WORLDPOS;
  float2 vUV       : TEXCOORD0;
  float3 vTangent  : TANGENT;
};

// Output control point
struct BEZIER_CONTROL_POINT
{
  float3 vPosition	: BEZIERPOS;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
  float Edges[4]        : SV_TessFactor;
  float Inside[2]       : SV_InsideTessFactor;

  float3 vTangent[4]    : TANGENT;
  float2 vUV[4]         : TEXCOORD;
  float3 vTanUCorner[4] : TANUCORNER;
  float3 vTanVCorner[4] : TANVCORNER;
  float4 vCWts          : TANWEIGHTS;
};

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT SubDToBezierConstantsHS(InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> ip, uint PatchID : SV_PrimitiveID) {
  HS_CONSTANT_DATA_OUTPUT Output;
  
  // Must initialize Edges and Inside; otherwise HLSL validation will fail.
  Output.Edges[0]  = 1.0;
  Output.Edges[1]  = 2.0;
  Output.Edges[2]  = 3.0;
  Output.Edges[3]  = 4.0;
  Output.Inside[0] = 5.0;
  Output.Inside[1] = 6.0;
  
  return Output;
}
