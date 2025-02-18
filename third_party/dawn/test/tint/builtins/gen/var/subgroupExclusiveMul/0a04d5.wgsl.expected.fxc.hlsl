SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupExclusiveMul_0a04d5() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = WavePrefixProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_0a04d5()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_0a04d5()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,16-39): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
