struct SB_RW {
  arg_0 : atomic<i32>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn atomicStore_d1e9a6() {
  atomicStore(&(sb_rw.arg_0), 1i);
}

@fragment
fn fragment_main() {
  atomicStore_d1e9a6();
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicStore_d1e9a6();
}
