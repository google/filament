SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupBroadcast_4a4334() {
  uint2 res = WaveReadLaneAt((1u).xx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_4a4334()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_4a4334()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-41): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
