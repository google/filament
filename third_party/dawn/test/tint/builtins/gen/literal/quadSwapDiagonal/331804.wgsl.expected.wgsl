enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn quadSwapDiagonal_331804() -> vec4<f32> {
  var res : vec4<f32> = quadSwapDiagonal(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_331804();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_331804();
}
