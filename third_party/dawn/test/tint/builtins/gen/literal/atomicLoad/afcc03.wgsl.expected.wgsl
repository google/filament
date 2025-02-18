@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

var<workgroup> arg_0 : atomic<i32>;

fn atomicLoad_afcc03() -> i32 {
  var res : i32 = atomicLoad(&(arg_0));
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atomicLoad_afcc03();
}
