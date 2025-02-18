SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupBroadcastFirst_e820d4() {
  int3 arg_0 = (int(1)).xxx;
  int3 res = WaveReadLaneFirst(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcastFirst_e820d4()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcastFirst_e820d4()));
}

FXC validation failure:
<scrubbed_path>(5,14-37): error X3004: undeclared identifier 'WaveReadLaneFirst'


tint executable returned error: exit status 1
