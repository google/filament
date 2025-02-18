//
// fragment_main
//
RWByteAddressBuffer sb_rw : register(u0);

void sb_rwatomicStore(uint offset, int value) {
  int ignored;
  sb_rw.InterlockedExchange(offset, value, ignored);
}


void atomicStore_d1e9a6() {
  int arg_1 = 0;
  arg_1 = 1;
  int x_20 = arg_1;
  sb_rwatomicStore(0u, x_20);
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
  int arg_1 = 0;
  arg_1 = 1;
  int x_20 = arg_1;
  sb_rwatomicStore(0u, x_20);
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
