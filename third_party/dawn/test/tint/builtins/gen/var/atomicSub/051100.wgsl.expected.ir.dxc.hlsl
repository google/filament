//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
int atomicSub_051100() {
  int arg_1 = int(1);
  int v = int(0);
  int v_1 = (int(0) - arg_1);
  sb_rw.InterlockedAdd(int(0u), v_1, v);
  int res = v;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicSub_051100()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
int atomicSub_051100() {
  int arg_1 = int(1);
  int v = int(0);
  int v_1 = (int(0) - arg_1);
  sb_rw.InterlockedAdd(int(0u), v_1, v);
  int res = v;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicSub_051100()));
}

