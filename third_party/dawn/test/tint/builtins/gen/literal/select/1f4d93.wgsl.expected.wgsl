fn select_1f4d93() {
  var res = select(vec2(1.0), vec2(1.0), vec2<bool>(true));
}

@fragment
fn fragment_main() {
  select_1f4d93();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_1f4d93();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  select_1f4d93();
  return out;
}
