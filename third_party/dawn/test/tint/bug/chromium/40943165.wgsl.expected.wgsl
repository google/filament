var<workgroup> W : mat2x2<f32>;

@compute @workgroup_size(1)
fn F(@builtin(local_invocation_index) mat2x2 : u32) {
  W[0] += 0;
}
