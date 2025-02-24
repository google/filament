SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupInclusiveMul_69326e() {
  float4 arg_0 = (1.0f).xxxx;
  float4 v = arg_0;
  float4 res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveMul_69326e()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveMul_69326e()));
}

FXC validation failure:
<scrubbed_path>(6,17-36): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
