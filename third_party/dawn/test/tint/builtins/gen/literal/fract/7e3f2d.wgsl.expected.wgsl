fn fract_7e3f2d() {
  var res = fract(vec4(1.25));
}

@fragment
fn fragment_main() {
  fract_7e3f2d();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_7e3f2d();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  fract_7e3f2d();
  return out;
}
