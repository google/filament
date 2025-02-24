SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupMin_8bb8c1() {
  uint2 res = WaveActiveMin((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupMin_8bb8c1()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMin_8bb8c1()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-36): error X3004: undeclared identifier 'WaveActiveMin'


tint executable returned error: exit status 1
