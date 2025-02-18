SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupInclusiveMul_7978b8() {
  float3 res = (WavePrefixProduct((1.0f).xxx) * (1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveMul_7978b8()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveMul_7978b8()));
}

FXC validation failure:
<scrubbed_path>(4,17-45): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
