SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupMul_fab258() {
  int4 res = WaveActiveProduct((int(1)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_fab258()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_fab258()));
}

FXC validation failure:
<scrubbed_path>(4,14-45): error X3004: undeclared identifier 'WaveActiveProduct'


tint executable returned error: exit status 1
