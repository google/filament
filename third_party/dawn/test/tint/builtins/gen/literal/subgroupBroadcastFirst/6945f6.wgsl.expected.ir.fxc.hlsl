SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupBroadcastFirst_6945f6() {
  float2 res = WaveReadLaneFirst((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcastFirst_6945f6()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcastFirst_6945f6()));
}

FXC validation failure:
<scrubbed_path>(4,16-43): error X3004: undeclared identifier 'WaveReadLaneFirst'


tint executable returned error: exit status 1
