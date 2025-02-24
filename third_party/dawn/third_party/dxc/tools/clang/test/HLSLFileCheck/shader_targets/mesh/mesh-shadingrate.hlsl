// RUN: %dxc -E main -T ms_6_5 %s | FileCheck %s

// CHECK: dx.op.storePrimitiveOutput.i32(i32 172,

struct MSvert {
  float4 pos : SV_Position;
};

struct MSprim
{
    uint id : SV_PrimitiveID;
    uint shadingRate : SV_ShadingRate;
};

[outputtopology("triangle")]
[numthreads(1,1,1)]
void main(
    in uint3 groupID : SV_GroupID, in uint3 threadInGroup : SV_GroupThreadID,
    out vertices MSvert verts[3],
    out primitives MSprim prims[1],
    out indices uint3 idx[1])
{
   SetMeshOutputCounts(3, 1);
   verts[0].pos = float4( 0.0f,   0.25f, 1.0f, 1.0f);
   verts[1].pos = float4( 0.25f, -0.25f, 1.0f, 1.0f);
   verts[2].pos = float4(-0.25f, -0.25f, 1.0f, 1.0f);
   prims[0].shadingRate = 0x5; // D3D12_SHADING_RATE_2X2
   idx[0]   = uint3(0,1,2);
}
