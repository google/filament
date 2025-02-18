fn modf_4bfced() {
  var res = modf(vec4<f32>(-(1.5f)));
}

@fragment
fn fragment_main() {
  modf_4bfced();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_4bfced();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  modf_4bfced();
  return out;
}
