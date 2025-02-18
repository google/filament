//
// fragment_main
//
RWByteAddressBuffer sb_rw : register(u0);

void sb_rwatomicStore(uint offset, int value) {
  int ignored;
  sb_rw.InterlockedExchange(offset, value, ignored);
}


void atomicStore_d1e9a6() {
  sb_rwatomicStore(0u, 1);
}

void fragment_main() {
  atomicStore_d1e9a6();
  return;
}
//
// compute_main
//
RWByteAddressBuffer sb_rw : register(u0);

void sb_rwatomicStore(uint offset, int value) {
  int ignored;
  sb_rw.InterlockedExchange(offset, value, ignored);
}


void atomicStore_d1e9a6() {
  sb_rwatomicStore(0u, 1);
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicStore_d1e9a6();
  return;
}
