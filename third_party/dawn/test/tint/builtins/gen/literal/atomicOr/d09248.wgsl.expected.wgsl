@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

var<workgroup> arg_0 : atomic<i32>;

fn atomicOr_d09248() -> i32 {
  var res : i32 = atomicOr(&(arg_0), 1i);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atomicOr_d09248();
}
