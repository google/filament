fn clamp_8b1eaa() {
  var res = clamp(vec3(1), vec3(1), vec3(1));
}

@fragment
fn fragment_main() {
  clamp_8b1eaa();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_8b1eaa();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  clamp_8b1eaa();
  return out;
}
