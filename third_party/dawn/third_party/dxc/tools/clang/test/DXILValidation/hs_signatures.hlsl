// For Validation::PSVSignatureTableReorder.

struct PSSceneIn
{
    float4 pos  : SV_Position;
    float2 tex  : TEXCOORD2;
    float3 norm : NORMAL3;
};

struct HSPerVertexData
{
    PSSceneIn v;
};

struct HSPerPatchData
{
	float	edges[2]	: SV_TessFactor;
    float       t               : PN_POSITION;
};

HSPerPatchData HSPerPatchFunc( const InputPatch< PSSceneIn, 2 > points, const OutputPatch< HSPerVertexData, 2 > opoints  )
{
    HSPerPatchData d;

    d.edges[0] = points[0].tex.x;
    d.edges[1] = opoints[1].v.tex.x;
    d.t = 2;

    return d;
}

// hull per-control point shader
[domain("isoline")]
[partitioning("fractional_odd")]
[outputtopology("line")]
[patchconstantfunc("HSPerPatchFunc")]
[outputcontrolpoints(2)]
HSPerVertexData main( const uint id : SV_OutputControlPointID,
                      const InputPatch< PSSceneIn, 2 > points )
{
    HSPerVertexData v;

    // Just forward the vertex
    v.v = points[ id ];

	return v;
}
