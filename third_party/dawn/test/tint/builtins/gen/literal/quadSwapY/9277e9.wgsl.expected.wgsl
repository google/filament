enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn quadSwapY_9277e9() -> f16 {
  var res : f16 = quadSwapY(1.0h);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_9277e9();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_9277e9();
}
