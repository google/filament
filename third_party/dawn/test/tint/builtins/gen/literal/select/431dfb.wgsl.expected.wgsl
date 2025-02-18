fn select_431dfb() {
  var res = select(vec2(1), vec2(1), vec2<bool>(true));
}

@fragment
fn fragment_main() {
  select_431dfb();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_431dfb();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  select_431dfb();
  return out;
}
