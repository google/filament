//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupAdd_7d1215() {
  float3 res = WaveActiveSum((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_7d1215()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupAdd_7d1215() {
  float3 res = WaveActiveSum((1.0f).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_7d1215()));
  return;
}
