// RUN: %dxc -T lib_6_3 -validator-version 1.8 -verify %s

// expected-warning@+1{{potential misuse of built-in constant 'RAYTRACING_PIPELINE_FLAG_ALLOW_OPACITY_MICROMAPS' in shader model lib_6_3; introduced in shader model 6.9}}
RaytracingPipelineConfig1 rpc = { 32, RAYTRACING_PIPELINE_FLAG_ALLOW_OPACITY_MICROMAPS };

RaytracingAccelerationStructure RTAS;
// DXR entry to test that restricted flags are diagnosed.
[shader("raygeneration")]
void main(void) {
	RayDesc rayDesc;

	// expected-warning@+2{{potential misuse of built-in constant 'RAY_FLAG_FORCE_OMM_2_STATE' in shader model lib_6_3; introduced in shader model 6.9}}
	// expected-warning@+1{{potential misuse of built-in constant 'RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS' in shader model lib_6_3; introduced in shader model 6.9}}
	RayQuery<RAY_FLAG_FORCE_OMM_2_STATE, RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS> rayQuery;
	// expected-warning@+1{{potential misuse of built-in constant 'RAY_FLAG_FORCE_OMM_2_STATE' in shader model lib_6_3; introduced in shader model 6.9}}
	rayQuery.TraceRayInline(RTAS, RAY_FLAG_FORCE_OMM_2_STATE, 2, rayDesc);
}
