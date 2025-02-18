fn transpose_ace596() {
  const arg_0 = mat3x2(1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
  var res = transpose(arg_0);
}

@fragment
fn fragment_main() {
  transpose_ace596();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_ace596();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  transpose_ace596();
  return out;
}
