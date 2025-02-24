//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupMin_a96a2e() {
  int arg_0 = int(1);
  int res = WaveActiveMin(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_a96a2e()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupMin_a96a2e() {
  int arg_0 = int(1);
  int res = WaveActiveMin(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_a96a2e()));
}

