enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn quadSwapX_a4e103() -> f16 {
  var res : f16 = quadSwapX(1.0h);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapX_a4e103();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapX_a4e103();
}
