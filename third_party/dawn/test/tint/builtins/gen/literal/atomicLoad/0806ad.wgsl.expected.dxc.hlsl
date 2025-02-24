//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

int sb_rwatomicLoad(uint offset) {
  int value = 0;
  sb_rw.InterlockedOr(offset, 0, value);
  return value;
}


int atomicLoad_0806ad() {
  int res = sb_rwatomicLoad(0u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicLoad_0806ad()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

int sb_rwatomicLoad(uint offset) {
  int value = 0;
  sb_rw.InterlockedOr(offset, 0, value);
  return value;
}


int atomicLoad_0806ad() {
  int res = sb_rwatomicLoad(0u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicLoad_0806ad()));
  return;
}
