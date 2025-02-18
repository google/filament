fn determinant_1bf6e7() {
  var res = determinant(mat2x2(1.0, 1.0, 1.0, 1.0));
}

@fragment
fn fragment_main() {
  determinant_1bf6e7();
}

@compute @workgroup_size(1)
fn compute_main() {
  determinant_1bf6e7();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  determinant_1bf6e7();
  return out;
}
