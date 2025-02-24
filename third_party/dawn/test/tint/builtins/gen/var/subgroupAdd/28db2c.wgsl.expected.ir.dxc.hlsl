//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupAdd_28db2c() {
  int4 arg_0 = (int(1)).xxxx;
  int4 res = WaveActiveSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupAdd_28db2c()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupAdd_28db2c() {
  int4 arg_0 = (int(1)).xxxx;
  int4 res = WaveActiveSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupAdd_28db2c()));
}

