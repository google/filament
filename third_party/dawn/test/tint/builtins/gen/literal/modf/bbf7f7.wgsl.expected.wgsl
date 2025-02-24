fn modf_bbf7f7() {
  var res = modf(-(1.5f));
}

@fragment
fn fragment_main() {
  modf_bbf7f7();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_bbf7f7();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  modf_bbf7f7();
  return out;
}
