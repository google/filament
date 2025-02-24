fn log_f60cc7() {
  const arg_0 = vec2(1.0);
  var res = log(arg_0);
}

@fragment
fn fragment_main() {
  log_f60cc7();
}

@compute @workgroup_size(1)
fn compute_main() {
  log_f60cc7();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  log_f60cc7();
  return out;
}
