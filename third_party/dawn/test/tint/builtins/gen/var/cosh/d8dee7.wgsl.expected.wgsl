fn cosh_d8dee7() {
  const arg_0 = vec4(0.0);
  var res = cosh(arg_0);
}

@fragment
fn fragment_main() {
  cosh_d8dee7();
}

@compute @workgroup_size(1)
fn compute_main() {
  cosh_d8dee7();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  cosh_d8dee7();
  return out;
}
