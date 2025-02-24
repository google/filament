enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn quadSwapX_8203ad() -> u32 {
  var arg_0 = 1u;
  var res : u32 = quadSwapX(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapX_8203ad();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapX_8203ad();
}
