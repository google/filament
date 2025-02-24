@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

var<workgroup> arg_0 : atomic<u32>;

fn atomicMin_69d383() -> u32 {
  var res : u32 = atomicMin(&(arg_0), 1u);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atomicMin_69d383();
}
