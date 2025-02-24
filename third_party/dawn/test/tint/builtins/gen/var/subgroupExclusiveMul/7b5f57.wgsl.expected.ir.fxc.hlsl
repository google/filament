SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupExclusiveMul_7b5f57() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = WavePrefixProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveMul_7b5f57()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveMul_7b5f57()));
}

FXC validation failure:
<scrubbed_path>(5,16-39): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
