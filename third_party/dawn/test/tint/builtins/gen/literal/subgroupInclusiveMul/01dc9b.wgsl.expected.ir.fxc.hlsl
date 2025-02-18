SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupInclusiveMul_01dc9b() {
  float2 res = (WavePrefixProduct((1.0f).xx) * (1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_01dc9b()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_01dc9b()));
}

FXC validation failure:
<scrubbed_path>(4,17-44): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
