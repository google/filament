SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupMax_7e81ea() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = WaveActiveMax(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupMax_7e81ea()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMax_7e81ea()));
}

FXC validation failure:
<scrubbed_path>(5,16-35): error X3004: undeclared identifier 'WaveActiveMax'


tint executable returned error: exit status 1
