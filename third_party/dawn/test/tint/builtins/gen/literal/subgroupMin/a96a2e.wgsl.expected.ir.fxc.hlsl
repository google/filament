SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupMin_a96a2e() {
  int res = WaveActiveMin(int(1));
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
<scrubbed_path>(4,13-33): error X3004: undeclared identifier 'WaveActiveMin'


tint executable returned error: exit status 1
