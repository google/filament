fn trunc_f0f1a1() {
  var res = trunc(vec4(1.5));
}

@fragment
fn fragment_main() {
  trunc_f0f1a1();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_f0f1a1();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  trunc_f0f1a1();
  return out;
}
