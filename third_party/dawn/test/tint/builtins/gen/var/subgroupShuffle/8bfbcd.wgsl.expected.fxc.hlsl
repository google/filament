SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupShuffle_8bfbcd() {
  int arg_0 = 1;
  int arg_1 = 1;
  int res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_8bfbcd()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_8bfbcd()));
  return;
}
FXC validation failure:
<scrubbed_path>(6,13-40): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
