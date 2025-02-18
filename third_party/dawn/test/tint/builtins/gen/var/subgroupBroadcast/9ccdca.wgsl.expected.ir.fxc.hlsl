SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupBroadcast_9ccdca() {
  int arg_0 = int(1);
  int res = WaveReadLaneAt(arg_0, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_9ccdca()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_9ccdca()));
}

FXC validation failure:
<scrubbed_path>(5,13-41): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
