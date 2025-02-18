SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupMax_1fc846() {
  float2 res = WaveActiveMax((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupMax_1fc846()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMax_1fc846()));
}

FXC validation failure:
<scrubbed_path>(4,16-39): error X3004: undeclared identifier 'WaveActiveMax'


tint executable returned error: exit status 1
