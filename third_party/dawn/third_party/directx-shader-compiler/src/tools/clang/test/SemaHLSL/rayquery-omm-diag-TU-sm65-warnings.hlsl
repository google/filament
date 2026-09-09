// RUN: %dxc -Wno-error-hlsl-rayquery-flags -Wno-error-hlsl-availability -T vs_6_5 -E main -verify %s

RaytracingAccelerationStructure RTAS;
void main(uint i : IDX, RayDesc rayDesc : RAYDESC) {

  // expected-warning@+3{{A non-zero value for the RayQueryFlags template argument requires shader model 6.9 or above.}}
  // expected-warning@+2{{When using 'RAY_FLAG_FORCE_OMM_2_STATE' in RayFlags, RayQueryFlags must have RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS set.}}
  // expected-warning@+1{{potential misuse of built-in constant 'RAY_FLAG_FORCE_OMM_2_STATE' in shader model vs_6_5; introduced in shader model 6.9}}
  RayQuery<RAY_FLAG_FORCE_OMM_2_STATE, 2> rayQuery0a;
  
}
