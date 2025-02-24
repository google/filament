fn select_4e60da() {
  var res = select(vec2(1.0), vec2(1.0), true);
}

@fragment
fn fragment_main() {
  select_4e60da();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_4e60da();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  select_4e60da();
  return out;
}
