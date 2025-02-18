fn fract_ed00ca() {
  var res = fract(vec2(1.25));
}

@fragment
fn fragment_main() {
  fract_ed00ca();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_ed00ca();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  fract_ed00ca();
  return out;
}
