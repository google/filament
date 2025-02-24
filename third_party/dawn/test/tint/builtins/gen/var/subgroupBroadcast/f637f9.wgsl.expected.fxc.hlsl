SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupBroadcast_f637f9() {
  int4 arg_0 = (1).xxxx;
  int4 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_f637f9()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_f637f9()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,14-38): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
