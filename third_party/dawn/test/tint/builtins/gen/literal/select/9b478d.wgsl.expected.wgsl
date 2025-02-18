fn select_9b478d() {
  var res = select(1, 1, true);
}

@fragment
fn fragment_main() {
  select_9b478d();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_9b478d();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  select_9b478d();
  return out;
}
