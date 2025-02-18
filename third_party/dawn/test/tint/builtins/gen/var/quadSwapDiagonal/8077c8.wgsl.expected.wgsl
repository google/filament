enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn quadSwapDiagonal_8077c8() -> vec2<f32> {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = quadSwapDiagonal(arg_0);
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
