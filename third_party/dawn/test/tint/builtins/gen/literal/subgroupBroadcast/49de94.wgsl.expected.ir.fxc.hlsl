SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupBroadcast_49de94() {
  uint res = WaveReadLaneAt(1u, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupBroadcast_49de94());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupBroadcast_49de94());
}

FXC validation failure:
<scrubbed_path>(4,14-39): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
