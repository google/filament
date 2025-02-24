enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn quadSwapDiagonal_2be5e7() -> f16 {
  var res : f16 = quadSwapDiagonal(1.0h);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_2be5e7();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_2be5e7();
}
