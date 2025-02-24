fn select_494051() {
  var res = select(1.0, 1.0, true);
}

@fragment
fn fragment_main() {
  select_494051();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_494051();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  select_494051();
  return out;
}
