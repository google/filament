SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupShuffle_2ee993() {
  int4 res = WaveReadLaneAt((1).xxxx, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_2ee993()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_2ee993()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-40): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
