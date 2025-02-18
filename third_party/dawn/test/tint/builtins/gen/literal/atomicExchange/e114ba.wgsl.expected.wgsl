@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

var<workgroup> arg_0 : atomic<i32>;

fn atomicExchange_e114ba() -> i32 {
  var res : i32 = atomicExchange(&(arg_0), 1i);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atomicExchange_e114ba();
}
