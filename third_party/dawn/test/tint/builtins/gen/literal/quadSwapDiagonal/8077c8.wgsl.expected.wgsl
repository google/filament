enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn quadSwapDiagonal_8077c8() -> vec2<f32> {
  var res : vec2<f32> = quadSwapDiagonal(vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_8077c8();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_8077c8();
}
