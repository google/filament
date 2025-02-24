fn select_4c4738() {
  var res = select(vec4(1), vec4(1), vec4<bool>(true));
}

@fragment
fn fragment_main() {
  select_4c4738();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_4c4738();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  select_4c4738();
  return out;
}
