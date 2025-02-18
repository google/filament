fn select_3a14be() {
  var res = select(vec2(1), vec2(1), true);
}

@fragment
fn fragment_main() {
  select_3a14be();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_3a14be();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  select_3a14be();
  return out;
}
