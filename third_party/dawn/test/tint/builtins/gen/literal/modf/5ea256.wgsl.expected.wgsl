fn modf_5ea256() {
  var res = modf(vec3<f32>(-(1.5f)));
}

@fragment
fn fragment_main() {
  modf_5ea256();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_5ea256();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  modf_5ea256();
  return out;
}
