fn fma_1f5084() {
  const arg_0 = vec2(1.0);
  const arg_1 = vec2(1.0);
  const arg_2 = vec2(1.0);
  var res = fma(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  fma_1f5084();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_1f5084();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  fma_1f5084();
  return out;
}
