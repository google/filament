enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn quadSwapDiagonal_e4bec8() -> vec3<f16> {
  var res : vec3<f16> = quadSwapDiagonal(vec3<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_e4bec8();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_e4bec8();
}
