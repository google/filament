SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupAdd_6587ff() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = WaveActiveSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupAdd_6587ff());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupAdd_6587ff());
}

FXC validation failure:
<scrubbed_path>(5,15-34): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
