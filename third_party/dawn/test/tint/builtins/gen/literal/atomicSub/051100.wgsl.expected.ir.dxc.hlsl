//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
int atomicSub_051100() {
  int v = int(0);
  sb_rw.InterlockedAdd(int(0u), (int(0) - int(1)), v);
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
  int v = int(0);
  sb_rw.InterlockedAdd(int(0u), (int(0) - int(1)), v);
  int res = v;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicSub_051100()));
}

