//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupMin_1de104() {
  int4 res = WaveActiveMin((int(1)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMin_1de104()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupMin_1de104() {
  int4 res = WaveActiveMin((int(1)).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMin_1de104()));
}

