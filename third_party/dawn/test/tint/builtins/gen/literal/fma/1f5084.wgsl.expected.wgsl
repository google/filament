fn fma_1f5084() {
  var res = fma(vec2(1.0), vec2(1.0), vec2(1.0));
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
