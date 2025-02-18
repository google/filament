fn fma_143d5d() {
  var res = fma(vec4(1.0), vec4(1.0), vec4(1.0));
}

@fragment
fn fragment_main() {
  fma_143d5d();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_143d5d();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  fma_143d5d();
  return out;
}
