//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
uint atomicLoad_fe6cc3() {
  uint v = 0u;
  sb_rw.InterlockedOr(0u, 0u, v);
  uint res = v;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, atomicLoad_fe6cc3());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
uint atomicLoad_fe6cc3() {
  uint v = 0u;
  sb_rw.InterlockedOr(0u, 0u, v);
  uint res = v;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, atomicLoad_fe6cc3());
}

