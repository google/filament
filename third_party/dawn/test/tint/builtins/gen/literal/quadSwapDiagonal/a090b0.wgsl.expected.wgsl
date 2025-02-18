enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn quadSwapDiagonal_a090b0() -> vec2<i32> {
  var res : vec2<i32> = quadSwapDiagonal(vec2<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_a090b0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_a090b0();
}
