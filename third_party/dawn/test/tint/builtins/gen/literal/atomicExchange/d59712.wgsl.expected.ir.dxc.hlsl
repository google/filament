//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
uint atomicExchange_d59712() {
  uint v = 0u;
  sb_rw.InterlockedExchange(0u, 1u, v);
  uint res = v;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, atomicExchange_d59712());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
uint atomicExchange_d59712() {
  uint v = 0u;
  sb_rw.InterlockedExchange(0u, 1u, v);
  uint res = v;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, atomicExchange_d59712());
}

