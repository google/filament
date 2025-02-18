SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupBroadcastFirst_5e5b6f() {
  uint3 res = WaveReadLaneFirst((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupBroadcastFirst_5e5b6f());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupBroadcastFirst_5e5b6f());
}

FXC validation failure:
<scrubbed_path>(4,15-41): error X3004: undeclared identifier 'WaveReadLaneFirst'


tint executable returned error: exit status 1
