SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupMin_2493ab() {
  uint res = WaveActiveMin(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_2493ab()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_2493ab()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-30): error X3004: undeclared identifier 'WaveActiveMin'


tint executable returned error: exit status 1
