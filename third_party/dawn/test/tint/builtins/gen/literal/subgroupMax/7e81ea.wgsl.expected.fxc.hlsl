SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupMax_7e81ea() {
  float3 res = WaveActiveMax((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupMax_7e81ea()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMax_7e81ea()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,16-40): error X3004: undeclared identifier 'WaveActiveMax'


tint executable returned error: exit status 1
