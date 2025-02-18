fn atanh_e431bb() {
  const arg_0 = vec4(0.5);
  var res = atanh(arg_0);
}

@fragment
fn fragment_main() {
  atanh_e431bb();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_e431bb();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  atanh_e431bb();
  return out;
}
