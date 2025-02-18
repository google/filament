fn modf_732aa6() {
  var res = modf(vec2(-(1.5)));
}

@fragment
fn fragment_main() {
  modf_732aa6();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_732aa6();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  modf_732aa6();
  return out;
}
