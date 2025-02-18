enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn quadSwapDiagonal_856536() -> vec3<u32> {
  var res : vec3<u32> = quadSwapDiagonal(vec3<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_856536();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_856536();
}
