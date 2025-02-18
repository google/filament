fn normalize_e7def8() {
  const arg_0 = vec3(1.0);
  var res = normalize(arg_0);
}

@fragment
fn fragment_main() {
  normalize_e7def8();
}

@compute @workgroup_size(1)
fn compute_main() {
  normalize_e7def8();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  normalize_e7def8();
  return out;
}
