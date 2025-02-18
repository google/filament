fn log2_a52bbb() {
  const arg_0 = vec4(1.0);
  var res = log2(arg_0);
}

@fragment
fn fragment_main() {
  log2_a52bbb();
}

@compute @workgroup_size(1)
fn compute_main() {
  log2_a52bbb();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  log2_a52bbb();
  return out;
}
