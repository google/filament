SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupShuffle_b4bbb7() {
  int3 arg_0 = (1).xxx;
  uint arg_1 = 1u;
  int3 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_b4bbb7()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffle_b4bbb7()));
  return;
}
FXC validation failure:
<scrubbed_path>(6,14-41): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
