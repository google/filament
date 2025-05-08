// RUN: %dxc -T vs_6_5 -E main -verify %s

// Test that at the call site of any TraceRayInline call, a default error
// warning is emitted that indicates the ray query object has the
// RAY_FLAG_FORCE_OMM_2_STATE set, but doesn't have 
// RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS set

RaytracingAccelerationStructure RTAS;
void main(RayDesc rayDesc : RAYDESC) : OUT {
  // expected-note@+1 2 {{RayQueryFlags declared here}}
  RayQuery<0> rayQuery; // implicitly, the second arg is 0.

  // expected-error@+2{{When using 'RAY_FLAG_FORCE_OMM_2_STATE' in RayFlags, RayQueryFlags must have RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS set.}}
  // expected-warning@+1{{potential misuse of built-in constant 'RAY_FLAG_FORCE_OMM_2_STATE' in shader model vs_6_5; introduced in shader model 6.9}}
  rayQuery.TraceRayInline(RTAS, RAY_FLAG_FORCE_OMM_2_STATE, 2, rayDesc);
  
  // expected-error@+1{{When using 'RAY_FLAG_FORCE_OMM_2_STATE' in RayFlags, RayQueryFlags must have RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS set.}}
  rayQuery.TraceRayInline(RTAS, 1024, 2, rayDesc);

  // expected-error@+1{{A non-zero value for the RayQueryFlags template argument requires shader model 6.9 or above.}}
  RayQuery<0, 1> rayQueryInvalid;
}
