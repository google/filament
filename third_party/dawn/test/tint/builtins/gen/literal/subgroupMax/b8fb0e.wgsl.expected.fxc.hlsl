SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupMax_b8fb0e() {
  uint2 res = WaveActiveMax((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupMax_b8fb0e()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMax_b8fb0e()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-36): error X3004: undeclared identifier 'WaveActiveMax'


tint executable returned error: exit status 1
