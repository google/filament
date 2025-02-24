//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
int atomicOr_8d96a0() {
  int arg_1 = int(1);
  int v = arg_1;
  int v_1 = int(0);
  sb_rw.InterlockedOr(int(0u), v, v_1);
  int res = v_1;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicOr_8d96a0()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
int atomicOr_8d96a0() {
  int arg_1 = int(1);
  int v = arg_1;
  int v_1 = int(0);
  sb_rw.InterlockedOr(int(0u), v, v_1);
  int res = v_1;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicOr_8d96a0()));
}

