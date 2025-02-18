//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
int atomicLoad_0806ad() {
  int v = int(0);
  sb_rw.InterlockedOr(int(0u), int(0), v);
  int res = v;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicLoad_0806ad()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
RWByteAddressBuffer sb_rw : register(u1);
int atomicLoad_0806ad() {
  int v = int(0);
  sb_rw.InterlockedOr(int(0u), int(0), v);
  int res = v;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicLoad_0806ad()));
}

