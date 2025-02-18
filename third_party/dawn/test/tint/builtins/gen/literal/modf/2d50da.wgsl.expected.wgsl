fn modf_2d50da() {
  var res = modf(vec2<f32>(-(1.5f)));
}

@fragment
fn fragment_main() {
  modf_2d50da();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_2d50da();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  modf_2d50da();
  return out;
}
