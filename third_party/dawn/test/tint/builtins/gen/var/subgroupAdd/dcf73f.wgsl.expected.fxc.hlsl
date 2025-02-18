SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupAdd_dcf73f() {
  float2 arg_0 = (1.0f).xx;
  float2 res = WaveActiveSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupAdd_dcf73f()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupAdd_dcf73f()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,16-35): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
