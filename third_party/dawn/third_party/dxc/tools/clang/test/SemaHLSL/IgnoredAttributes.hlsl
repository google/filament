// RUN: %dxc -T lib_6_8 -verify %s
// test that clipplanes emits an error when no shader attribute is present,
// to warn the user that the deprecated behavior of inferring the shader kind
// is no longer taking place.

cbuffer ClipPlaneConstantBuffer 
{
       float4 clipPlane1;
       float4 clipPlane2;
};

struct OutputType{
	float4 f4 : SV_Position;
};


[clipplanes(clipPlane1,clipPlane2)] /* expected-warning{{attribute 'clipplanes' ignored without accompanying shader attribute}} */
OutputType clipplanesmain()
{
	float4 outputData = clipPlane1 + clipPlane2;
	OutputType output = {outputData};
	return output;
}

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

HSPerPatchData PatchFoo( const InputPatch< PSSceneIn, 3 > points, OutputPatch<HSPerVertexData, 3> outp)
{
    HSPerPatchData d;

    d.edges[ 0 ] = 1;
    d.edges[ 1 ] = 1;
    d.edges[ 2 ] = 1;
    d.inside = 1;

    return d;
}

[domain("quad")] /* expected-warning{{attribute 'domain' ignored without accompanying shader attribute}} */
[partitioning("integer")] /* expected-warning{{attribute 'partitioning' ignored without accompanying shader attribute}} */
[outputtopology("triangle_cw")] /* expected-warning{{attribute 'outputtopology' ignored without accompanying shader attribute}} */
[outputcontrolpoints(16)] /* expected-warning{{attribute 'outputcontrolpoints' ignored without accompanying shader attribute}} */
[patchconstantfunc("PatchFoo")] /* expected-warning{{attribute 'patchconstantfunc' ignored without accompanying shader attribute}} */
void HSMain( 
              uint i : SV_OutputControlPointID,
              uint PatchID : SV_PrimitiveID )
{
    
}

[earlydepthstencil] /* expected-warning{{attribute 'earlydepthstencil' ignored without accompanying shader attribute}} */
float4 EDSmain() : SV_Target0 {
	float4 x = {2.0,2.0,2.0,2.0};;
    	return x;
}

[instance(1)] /* expected-warning{{attribute 'instance' ignored without accompanying shader attribute}} */ 
int instance_fn() { return 1; } 

[maxtessfactor(1)] /* expected-warning{{attribute 'maxtessfactor' ignored without accompanying shader attribute}} */ 
int maxtessfactor_fn() { return 1; }

[numthreads(4,4,4)] /* expected-warning{{attribute 'numthreads' ignored without accompanying shader attribute}} */ 
int numthreads_fn() { return 1; }   

[RootSignature("RootFlags(CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED)")] /* expected-warning{{attribute 'RootSignature' ignored without accompanying shader attribute}} */ 
void rootsig_fn(){}

[maxvertexcount(12)] /* expected-warning{{attribute 'maxvertexcount' ignored without accompanying shader attribute}} */ 
void maxvertexcount_fn(){}

[wavesize(4,32,16)] /* expected-warning{{attribute 'wavesize' ignored without accompanying shader attribute}} */ 
void wavesize_fn(){}

[nodelaunch("thread")] /* expected-warning{{attribute 'nodelaunch' ignored without accompanying shader attribute}} */ 
void nodelaunch_fn(){}

[nodeisprogramentry] /* expected-warning{{attribute 'nodeisprogramentry' ignored without accompanying shader attribute}} */ 
void nodeisprogramentry_fn(){}

[nodeid("launch")] /* expected-warning{{attribute 'nodeid' ignored without accompanying shader attribute}} */ 
void nodeid_fn(){}

[NodeLocalRootArgumentsTableIndex(2)] /* expected-warning{{attribute 'nodelocalrootargumentstableindex' ignored without accompanying shader attribute}} */ 
void NodeLocalRootArgumentsTableIndex_fn(){}

[NodeShareInputOf("launch")] /* expected-warning{{attribute 'nodeshareinputof' ignored without accompanying shader attribute}} */ 
void NodeShareInputOf_fn(){}

[NodeDispatchGrid(1,2,3)] /* expected-warning{{attribute 'nodedispatchgrid' ignored without accompanying shader attribute}} */ 
void NodeDispatchGrid_fn(){}

[NodeMaxDispatchGrid(1,2,3)] /* expected-warning{{attribute 'nodemaxdispatchgrid' ignored without accompanying shader attribute}} */ 
void NodeMaxDispatchGrid_fn(){}

[NodeMaxRecursionDepth(12)] /* expected-warning{{attribute 'nodemaxrecursiondepth' ignored without accompanying shader attribute}} */ 
void NodeMaxRecursionDepth_fn(){}