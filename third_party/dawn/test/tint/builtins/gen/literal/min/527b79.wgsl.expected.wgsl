fn min_527b79() {
  var res = min(vec2(1), vec2(1));
}

@fragment
fn fragment_main() {
  min_527b79();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_527b79();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  min_527b79();
  return out;
}
