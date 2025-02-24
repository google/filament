fn log_b8088d() {
  var res = log(vec3(1.0));
}

@fragment
fn fragment_main() {
  log_b8088d();
}

@compute @workgroup_size(1)
fn compute_main() {
  log_b8088d();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  log_b8088d();
  return out;
}
