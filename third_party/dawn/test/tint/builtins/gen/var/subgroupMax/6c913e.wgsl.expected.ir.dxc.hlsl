//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupMax_6c913e() {
  int arg_0 = int(1);
  int res = WaveActiveMax(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMax_6c913e()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupMax_6c913e() {
  int arg_0 = int(1);
  int res = WaveActiveMax(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMax_6c913e()));
}

