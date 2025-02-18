fn tanh_c48aa6() {
  const arg_0 = vec2(1.0);
  var res = tanh(arg_0);
}

@fragment
fn fragment_main() {
  tanh_c48aa6();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_c48aa6();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  tanh_c48aa6();
  return out;
}
