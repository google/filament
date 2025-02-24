SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupAdd_1eb429() {
  int2 res = WaveActiveSum((1).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupAdd_1eb429()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupAdd_1eb429()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-34): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
