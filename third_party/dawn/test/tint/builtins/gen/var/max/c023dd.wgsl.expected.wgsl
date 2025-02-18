fn max_c023dd() {
  const arg_0 = 1.0;
  const arg_1 = 1.0;
  var res = max(arg_0, arg_1);
}

@fragment
fn fragment_main() {
  max_c023dd();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_c023dd();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  max_c023dd();
  return out;
}
