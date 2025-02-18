SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupMin_2493ab() {
  uint arg_0 = 1u;
  uint res = WaveActiveMin(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupMin_2493ab());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupMin_2493ab());
}

FXC validation failure:
<scrubbed_path>(5,14-33): error X3004: undeclared identifier 'WaveActiveMin'


tint executable returned error: exit status 1
