fn clamp_9d731c() {
  var res = clamp(vec2(1.0), vec2(1.0), vec2(1.0));
}

@fragment
fn fragment_main() {
  clamp_9d731c();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_9d731c();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  clamp_9d731c();
  return out;
}
