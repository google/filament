//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupAll_c962bd() {
  bool arg_0 = true;
  bool res = WaveActiveAllTrue(arg_0);
  return ((all((res == false))) ? (int(1)) : (int(0)));
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupAll_c962bd()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupAll_c962bd() {
  bool arg_0 = true;
  bool res = WaveActiveAllTrue(arg_0);
  return ((all((res == false))) ? (int(1)) : (int(0)));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAll_c962bd()));
}

