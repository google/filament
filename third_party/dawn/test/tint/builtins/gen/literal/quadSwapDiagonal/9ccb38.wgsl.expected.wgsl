enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn quadSwapDiagonal_9ccb38() -> i32 {
  var res : i32 = quadSwapDiagonal(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_9ccb38();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_9ccb38();
}
