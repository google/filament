SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupShuffle_54f328() {
  uint arg_0 = 1u;
  int arg_1 = 1;
  uint res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_54f328()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_54f328()));
  return;
}
FXC validation failure:
<scrubbed_path>(6,14-41): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
