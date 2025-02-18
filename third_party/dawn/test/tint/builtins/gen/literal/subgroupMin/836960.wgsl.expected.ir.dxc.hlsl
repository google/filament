//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupMin_836960() {
  float3 res = WaveActiveMin((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupMin_836960()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupMin_836960() {
  float3 res = WaveActiveMin((1.0f).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMin_836960()));
}

