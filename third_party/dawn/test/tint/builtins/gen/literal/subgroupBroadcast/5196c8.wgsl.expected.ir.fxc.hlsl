SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupBroadcast_5196c8() {
  float2 res = WaveReadLaneAt((1.0f).xx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_5196c8()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_5196c8()));
}

FXC validation failure:
<scrubbed_path>(4,16-44): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
