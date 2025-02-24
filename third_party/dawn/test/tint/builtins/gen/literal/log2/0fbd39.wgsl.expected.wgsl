fn log2_0fbd39() {
  var res = log2(vec3(1.0));
}

@fragment
fn fragment_main() {
  log2_0fbd39();
}

@compute @workgroup_size(1)
fn compute_main() {
  log2_0fbd39();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  log2_0fbd39();
  return out;
}
