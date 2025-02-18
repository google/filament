//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
uint atomicAnd_85a8d9() {
  uint arg_1 = 1u;
  uint v = 0u;
  sb_rw.InterlockedAnd(0u, arg_1, v);
  uint res = v;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, atomicAnd_85a8d9());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
uint atomicAnd_85a8d9() {
  uint arg_1 = 1u;
  uint v = 0u;
  sb_rw.InterlockedAnd(0u, arg_1, v);
  uint res = v;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, atomicAnd_85a8d9());
}

