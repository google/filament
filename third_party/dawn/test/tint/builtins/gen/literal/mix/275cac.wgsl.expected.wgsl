fn mix_275cac() {
  var res = mix(vec4(1.0), vec4(1.0), 1.0);
}

@fragment
fn fragment_main() {
  mix_275cac();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_275cac();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  mix_275cac();
  return out;
}
