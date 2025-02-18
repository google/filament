SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupMin_836960() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = WaveActiveMin(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupMin_836960()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMin_836960()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,16-35): error X3004: undeclared identifier 'WaveActiveMin'


tint executable returned error: exit status 1
