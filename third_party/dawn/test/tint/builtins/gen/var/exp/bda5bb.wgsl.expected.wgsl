fn exp_bda5bb() {
  const arg_0 = vec3(1.0);
  var res = exp(arg_0);
}

@fragment
fn fragment_main() {
  exp_bda5bb();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp_bda5bb();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  exp_bda5bb();
  return out;
}
