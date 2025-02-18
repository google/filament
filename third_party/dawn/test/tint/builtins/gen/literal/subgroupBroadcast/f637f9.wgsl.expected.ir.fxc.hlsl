SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupBroadcast_f637f9() {
  int4 res = WaveReadLaneAt((int(1)).xxxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_f637f9()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_f637f9()));
}

FXC validation failure:
<scrubbed_path>(4,14-46): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
