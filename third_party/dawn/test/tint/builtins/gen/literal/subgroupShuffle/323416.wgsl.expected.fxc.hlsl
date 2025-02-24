SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupShuffle_323416() {
  int2 res = WaveReadLaneAt((1).xx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffle_323416()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffle_323416()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-39): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
