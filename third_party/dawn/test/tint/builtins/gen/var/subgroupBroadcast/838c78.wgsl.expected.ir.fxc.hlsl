SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupBroadcast_838c78() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = WaveReadLaneAt(arg_0, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_838c78()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_838c78()));
}

FXC validation failure:
<scrubbed_path>(5,16-44): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
