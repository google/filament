//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
uint atomicSub_15bfc9() {
  uint v = 0u;
  sb_rw.InterlockedAdd(0u, (0u - 1u), v);
  uint res = v;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, atomicSub_15bfc9());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
uint atomicSub_15bfc9() {
  uint v = 0u;
  sb_rw.InterlockedAdd(0u, (0u - 1u), v);
  uint res = v;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, atomicSub_15bfc9());
}

