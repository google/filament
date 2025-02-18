//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupMin_2493ab() {
  uint arg_0 = 1u;
  uint res = WaveActiveMin(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupMin_2493ab());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupMin_2493ab() {
  uint arg_0 = 1u;
  uint res = WaveActiveMin(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupMin_2493ab());
}

