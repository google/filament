SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupBroadcast_cd7aa1() {
  float2 res = WaveReadLaneAt((1.0f).xx, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_cd7aa1()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_cd7aa1()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,16-43): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
