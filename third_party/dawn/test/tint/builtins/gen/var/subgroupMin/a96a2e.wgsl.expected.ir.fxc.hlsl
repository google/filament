SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupMin_a96a2e() {
  int arg_0 = int(1);
  int res = WaveActiveMin(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_a96a2e()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_a96a2e()));
}

FXC validation failure:
<scrubbed_path>(5,13-32): error X3004: undeclared identifier 'WaveActiveMin'


tint executable returned error: exit status 1
