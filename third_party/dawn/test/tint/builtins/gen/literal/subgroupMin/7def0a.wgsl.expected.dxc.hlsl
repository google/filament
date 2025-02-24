//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float subgroupMin_7def0a() {
  float res = WaveActiveMin(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_7def0a()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float subgroupMin_7def0a() {
  float res = WaveActiveMin(1.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_7def0a()));
  return;
}
