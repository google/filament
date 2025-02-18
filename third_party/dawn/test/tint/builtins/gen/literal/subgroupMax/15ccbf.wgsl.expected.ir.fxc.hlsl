SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupMax_15ccbf() {
  uint4 res = WaveActiveMax((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupMax_15ccbf());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupMax_15ccbf());
}

FXC validation failure:
<scrubbed_path>(4,15-38): error X3004: undeclared identifier 'WaveActiveMax'


tint executable returned error: exit status 1
