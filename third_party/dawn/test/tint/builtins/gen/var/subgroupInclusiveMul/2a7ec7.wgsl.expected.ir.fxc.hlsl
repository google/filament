SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float subgroupInclusiveMul_2a7ec7() {
  float arg_0 = 1.0f;
  float v = arg_0;
  float res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveMul_2a7ec7()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveMul_2a7ec7()));
}

FXC validation failure:
<scrubbed_path>(6,16-35): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
