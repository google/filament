SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupMul_f78398() {
  float2 res = WaveActiveProduct((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupMul_f78398()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMul_f78398()));
}

FXC validation failure:
<scrubbed_path>(4,16-43): error X3004: undeclared identifier 'WaveActiveProduct'


tint executable returned error: exit status 1
