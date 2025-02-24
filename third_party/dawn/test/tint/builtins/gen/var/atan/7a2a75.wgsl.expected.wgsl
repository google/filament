fn atan_7a2a75() {
  const arg_0 = 1.0;
  var res = atan(arg_0);
}

@fragment
fn fragment_main() {
  atan_7a2a75();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_7a2a75();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  atan_7a2a75();
  return out;
}
