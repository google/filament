fn determinant_c8251d() {
  const arg_0 = mat3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
  var res = determinant(arg_0);
}

@fragment
fn fragment_main() {
  determinant_c8251d();
}

@compute @workgroup_size(1)
fn compute_main() {
  determinant_c8251d();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  determinant_c8251d();
  return out;
}
