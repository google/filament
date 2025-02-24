fn trunc_c12555() {
  const arg_0 = vec2(1.5);
  var res = trunc(arg_0);
}

@fragment
fn fragment_main() {
  trunc_c12555();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_c12555();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  trunc_c12555();
  return out;
}
