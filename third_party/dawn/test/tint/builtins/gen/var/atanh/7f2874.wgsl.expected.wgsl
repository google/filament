fn atanh_7f2874() {
  const arg_0 = vec3(0.5);
  var res = atanh(arg_0);
}

@fragment
fn fragment_main() {
  atanh_7f2874();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_7f2874();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  atanh_7f2874();
  return out;
}
