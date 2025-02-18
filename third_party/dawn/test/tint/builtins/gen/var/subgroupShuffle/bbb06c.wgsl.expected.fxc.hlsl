SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupShuffle_bbb06c() {
  int2 arg_0 = (1).xx;
  int arg_1 = 1;
  int2 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffle_bbb06c()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupShuffle_bbb06c()));
  return;
}
FXC validation failure:
<scrubbed_path>(6,14-41): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
