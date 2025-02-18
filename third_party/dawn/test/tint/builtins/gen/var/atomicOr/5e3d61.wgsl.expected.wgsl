@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

var<workgroup> arg_0 : atomic<u32>;

fn atomicOr_5e3d61() -> u32 {
  var arg_1 = 1u;
  var res : u32 = atomicOr(&(arg_0), arg_1);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atomicOr_5e3d61();
}
