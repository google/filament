SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupShuffle_824702() {
  int3 res = WaveReadLaneAt((int(1)).xxx, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_824702()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_824702()));
}

FXC validation failure:
<scrubbed_path>(4,14-49): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
