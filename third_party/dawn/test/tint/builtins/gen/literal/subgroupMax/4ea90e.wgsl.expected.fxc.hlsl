SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupMax_4ea90e() {
  int3 res = WaveActiveMax((1).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupMax_4ea90e()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMax_4ea90e()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-35): error X3004: undeclared identifier 'WaveActiveMax'


tint executable returned error: exit status 1
