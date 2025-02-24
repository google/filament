@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

var<workgroup> arg_0 : atomic<u32>;

fn atomicExchange_0a5dca() -> u32 {
  var res : u32 = atomicExchange(&(arg_0), 1u);
  return res;
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atomicExchange_0a5dca();
}
