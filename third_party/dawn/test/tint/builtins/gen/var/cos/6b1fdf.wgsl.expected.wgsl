fn cos_6b1fdf() {
  const arg_0 = vec3(0.0);
  var res = cos(arg_0);
}

@fragment
fn fragment_main() {
  cos_6b1fdf();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_6b1fdf();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  cos_6b1fdf();
  return out;
}
