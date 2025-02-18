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
  return;
}

void fragment_main_1() {
  atomicStore_d1e9a6();
  return;
}

void fragment_main() {
  fragment_main_1();
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
  return;
}

void compute_main_1() {
  atomicStore_d1e9a6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
