fn sinh_9c1092() {
  var res = sinh(vec2(1.0));
}

@fragment
fn fragment_main() {
  sinh_9c1092();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_9c1092();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  sinh_9c1092();
  return out;
}
