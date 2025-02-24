fn clamp_5cf700() {
  var res = clamp(vec3(1.0), vec3(1.0), vec3(1.0));
}

@fragment
fn fragment_main() {
  clamp_5cf700();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_5cf700();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  clamp_5cf700();
  return out;
}
