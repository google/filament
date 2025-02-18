//
// fragment_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicStore_d1e9a6() {
  int arg_1 = int(1);
  int v = arg_1;
  int v_1 = int(0);
  sb_rw.InterlockedExchange(int(0u), v, v_1);
}

void fragment_main() {
  atomicStore_d1e9a6();
}

//
// compute_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicStore_d1e9a6() {
  int arg_1 = int(1);
  int v = arg_1;
  int v_1 = int(0);
  sb_rw.InterlockedExchange(int(0u), v, v_1);
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicStore_d1e9a6();
}

