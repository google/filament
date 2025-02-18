SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupAdd_7d1215() {
  float3 res = WaveActiveSum((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_7d1215()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_7d1215()));
}

FXC validation failure:
<scrubbed_path>(4,16-40): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
