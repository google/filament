SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupMax_a3d5f7() {
  int4 res = WaveActiveMax((1).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMax_a3d5f7()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMax_a3d5f7()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-36): error X3004: undeclared identifier 'WaveActiveMax'


tint executable returned error: exit status 1
