struct SB_RW {
  arg_0 : atomic<i32>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn atomicStore_d1e9a6() {
  var arg_1 = 1i;
  atomicStore(&(sb_rw.arg_0), arg_1);
}

@fragment
fn fragment_main() {
  atomicStore_d1e9a6();
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicStore_d1e9a6();
}
