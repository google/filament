//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupAny_cddda0() {
  bool res = WaveActiveAnyTrue(true);
  return ((all((res == false))) ? (int(1)) : (int(0)));
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupAny_cddda0()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupAny_cddda0() {
  bool res = WaveActiveAnyTrue(true);
  return ((all((res == false))) ? (int(1)) : (int(0)));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAny_cddda0()));
}

