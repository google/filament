fn fma_466442() {
  var res = fma(1.0, 1.0, 1.0);
}

@fragment
fn fragment_main() {
  fma_466442();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_466442();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  fma_466442();
  return out;
}
