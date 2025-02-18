SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupMax_23f502() {
  uint3 res = WaveActiveMax((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupMax_23f502()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMax_23f502()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-37): error X3004: undeclared identifier 'WaveActiveMax'


tint executable returned error: exit status 1
