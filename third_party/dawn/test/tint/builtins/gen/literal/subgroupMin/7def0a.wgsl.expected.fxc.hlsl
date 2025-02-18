SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupMin_7def0a() {
  float res = WaveActiveMin(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_7def0a()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_7def0a()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-33): error X3004: undeclared identifier 'WaveActiveMin'


tint executable returned error: exit status 1
