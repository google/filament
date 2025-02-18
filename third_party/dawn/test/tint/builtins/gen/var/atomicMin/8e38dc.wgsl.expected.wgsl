@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

struct SB_RW {
  arg_0 : atomic<i32>,
}

@group(0) @binding(1) var<storage, read_write> sb_rw : SB_RW;

fn atomicMin_8e38dc() -> i32 {
  var arg_1 = 1i;
  var res : i32 = atomicMin(&(sb_rw.arg_0), arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = atomicMin_8e38dc();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atomicMin_8e38dc();
}
