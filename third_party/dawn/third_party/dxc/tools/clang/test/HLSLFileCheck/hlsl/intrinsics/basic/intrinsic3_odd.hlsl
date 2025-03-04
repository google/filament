// RUN: %dxc -E main -T hs_6_0 %s | FileCheck %s

// CHECK: FMax
// CHECK: FMin
// CHECK: storePatchConstant
// CHECK: outputControlPointID
// CHECK: loadInput
// CHECK: storeOutput

float v;

float4  Quad2DRawEdgeFactors;
float2  Quad2DInsideScale;

float   IsolineRawDetailFactor;
float   IsolineRawDensityFactor;

float4  QuadRawEdgeFactors;
float   QuadInsideScale;

float3  TriRawEdgeFactors;
float   TriInsideScale;

struct PSSceneIn
{
    float4 pos  : SV_Position;
    float2 tex  : TEXCOORD0;
    float3 norm : NORMAL;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Simple forwarding Tessellation shaders

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

HSPerPatchData HSPerPatchFunc( const InputPatch< PSSceneIn, 3 > points )
{
  float4 edgeFactor;
  float2 insideFactor;
  
  float4 Quad2DRoundedEdgeTessFactors;
  float2 Quad2DRoundedInsideTessFactors;
  float2 Quad2DUnroundedInsideTessFactors;
  Process2DQuadTessFactorsAvg(
     Quad2DRawEdgeFactors,
     Quad2DInsideScale,
     Quad2DRoundedEdgeTessFactors,
	 Quad2DRoundedInsideTessFactors,
	 Quad2DUnroundedInsideTessFactors
  );
  
  edgeFactor = Quad2DRoundedEdgeTessFactors;
  insideFactor = Quad2DRoundedInsideTessFactors;
  insideFactor += Quad2DUnroundedInsideTessFactors;
  

  Process2DQuadTessFactorsMax(
     Quad2DRawEdgeFactors,
     Quad2DInsideScale,
     Quad2DRoundedEdgeTessFactors,
	 Quad2DRoundedInsideTessFactors,
	 Quad2DUnroundedInsideTessFactors
  );  

  edgeFactor += Quad2DRoundedEdgeTessFactors;
  insideFactor += Quad2DRoundedInsideTessFactors;
  insideFactor += Quad2DUnroundedInsideTessFactors;  
  
  Process2DQuadTessFactorsMin(
     Quad2DRawEdgeFactors,
     Quad2DInsideScale,
     Quad2DRoundedEdgeTessFactors,
	 Quad2DRoundedInsideTessFactors,
	 Quad2DUnroundedInsideTessFactors
  );

  edgeFactor += Quad2DRoundedEdgeTessFactors;
  insideFactor += Quad2DRoundedInsideTessFactors;
  insideFactor += Quad2DUnroundedInsideTessFactors;    
 
  float IsolineRoundedDetailFactor;
  float IsolineRoundedDensityFactor;
ProcessIsolineTessFactors(
  IsolineRawDetailFactor,
  IsolineRawDensityFactor,
  IsolineRoundedDetailFactor,
  IsolineRoundedDensityFactor
);

  edgeFactor += IsolineRoundedDetailFactor;
  insideFactor += IsolineRoundedDensityFactor; 

  float4 QuadRoundedEdgeTessFactors;
  float2 QuadRoundedInsideTessFactors = 9;
  float2 QuadUnroundedInsideTessFactors;

ProcessQuadTessFactorsAvg(
 QuadRawEdgeFactors,
   QuadInsideScale,
   QuadRoundedEdgeTessFactors,
    QuadRoundedInsideTessFactors,
   QuadUnroundedInsideTessFactors
);

  edgeFactor += QuadRoundedEdgeTessFactors;
  insideFactor += QuadRoundedInsideTessFactors;
  insideFactor += QuadUnroundedInsideTessFactors;  

ProcessQuadTessFactorsMax(
 QuadRawEdgeFactors,
   QuadInsideScale,
   QuadRoundedEdgeTessFactors,
    QuadRoundedInsideTessFactors,
   QuadUnroundedInsideTessFactors
);

  edgeFactor += QuadRoundedEdgeTessFactors;
  insideFactor += QuadRoundedInsideTessFactors;
  insideFactor += QuadUnroundedInsideTessFactors;
  
ProcessQuadTessFactorsMin(
 QuadRawEdgeFactors,
   QuadInsideScale,
   QuadRoundedEdgeTessFactors,
    QuadRoundedInsideTessFactors,
   QuadUnroundedInsideTessFactors
);

  edgeFactor += QuadRoundedEdgeTessFactors;
  insideFactor += QuadRoundedInsideTessFactors;
  insideFactor += QuadUnroundedInsideTessFactors;

    float3 TriRoundedEdgeTessFactors;
    float TriRoundedInsideTessFactor;
    float TriUnroundedInsideTessFactor;

ProcessTriTessFactorsAvg(
 TriRawEdgeFactors,
   TriInsideScale,
   TriRoundedEdgeTessFactors,
   TriRoundedInsideTessFactor,
   TriUnroundedInsideTessFactor
);

  edgeFactor += TriRoundedEdgeTessFactors.xyzz;
  insideFactor += TriRoundedInsideTessFactor;
  insideFactor += TriUnroundedInsideTessFactor;

ProcessTriTessFactorsMax(
 TriRawEdgeFactors,
   TriInsideScale,
   TriRoundedEdgeTessFactors,
   TriRoundedInsideTessFactor,
   TriUnroundedInsideTessFactor
);

  edgeFactor += TriRoundedEdgeTessFactors.xyzz;
  insideFactor += TriRoundedInsideTessFactor;
  insideFactor += TriUnroundedInsideTessFactor;

ProcessTriTessFactorsMin(
 TriRawEdgeFactors,
   TriInsideScale,
   TriRoundedEdgeTessFactors,
   TriRoundedInsideTessFactor,
   TriUnroundedInsideTessFactor
);

  edgeFactor += TriRoundedEdgeTessFactors.xyzz;
  insideFactor += TriRoundedInsideTessFactor;
  insideFactor += TriUnroundedInsideTessFactor;

    HSPerPatchData d;

    d.edges[ 0 ] = edgeFactor.x;
    d.edges[ 1 ] = edgeFactor.y;
    d.edges[ 2 ] = edgeFactor.z + edgeFactor.w;
    d.inside = insideFactor.x + insideFactor.y;

    return d;
}

// hull per-control point shader
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HSPerPatchFunc")]
[outputcontrolpoints(3)]
HSPerVertexData main( const uint id : SV_OutputControlPointID,
                               const InputPatch< PSSceneIn, 3 > points )
{
    HSPerVertexData v;

    // Just forward the vertex
    v.v = points[ id ];

	return v;
}
