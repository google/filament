SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupBroadcast_8855b2() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = WaveReadLaneAt(arg_0, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_8855b2()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcast_8855b2()));
}

FXC validation failure:
<scrubbed_path>(5,16-44): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
