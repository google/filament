fn select_494051() {
  const arg_0 = 1.0;
  const arg_1 = 1.0;
  var arg_2 = true;
  var res = select(arg_0, arg_1, arg_2);
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
