SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float4 subgroupMin_bbd9b0() {
  float4 res = WaveActiveMin((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMin_bbd9b0()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMin_bbd9b0()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,16-41): error X3004: undeclared identifier 'WaveActiveMin'


tint executable returned error: exit status 1
