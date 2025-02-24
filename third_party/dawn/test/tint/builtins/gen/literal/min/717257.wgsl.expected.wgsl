fn min_717257() {
  var res = min(vec2(1.0), vec2(1.0));
}

@fragment
fn fragment_main() {
  min_717257();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_717257();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  min_717257();
  return out;
}
