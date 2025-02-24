enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn quadSwapY_0c4938() -> u32 {
  var arg_0 = 1u;
  var res : u32 = quadSwapY(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_0c4938();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_0c4938();
}
