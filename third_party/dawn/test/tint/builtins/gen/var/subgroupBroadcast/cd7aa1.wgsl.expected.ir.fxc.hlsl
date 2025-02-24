SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupBroadcast_cd7aa1() {
  float2 arg_0 = (1.0f).xx;
  float2 res = WaveReadLaneAt(arg_0, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_cd7aa1()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_cd7aa1()));
}

FXC validation failure:
<scrubbed_path>(5,16-44): error X3004: undeclared identifier 'WaveReadLaneAt'


tint executable returned error: exit status 1
