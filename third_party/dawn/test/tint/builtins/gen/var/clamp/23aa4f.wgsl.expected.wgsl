fn clamp_23aa4f() {
  const arg_0 = 1.0;
  const arg_1 = 1.0;
  const arg_2 = 1.0;
  var res = clamp(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  clamp_23aa4f();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_23aa4f();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  clamp_23aa4f();
  return out;
}
