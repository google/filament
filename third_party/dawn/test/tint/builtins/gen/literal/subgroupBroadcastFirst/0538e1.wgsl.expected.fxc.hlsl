SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupBroadcastFirst_0538e1() {
  float res = WaveReadLaneFirst(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcastFirst_0538e1()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcastFirst_0538e1()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-37): error X3004: undeclared identifier 'WaveReadLaneFirst'


tint executable returned error: exit status 1
