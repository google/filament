fn tan_ae26ae() {
  const arg_0 = vec3(1.0);
  var res = tan(arg_0);
}

@fragment
fn fragment_main() {
  tan_ae26ae();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_ae26ae();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  tan_ae26ae();
  return out;
}
