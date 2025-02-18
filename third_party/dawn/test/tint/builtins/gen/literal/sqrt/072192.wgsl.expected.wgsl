fn sqrt_072192() {
  var res = sqrt(vec3(1.0));
}

@fragment
fn fragment_main() {
  sqrt_072192();
}

@compute @workgroup_size(1)
fn compute_main() {
  sqrt_072192();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  sqrt_072192();
  return out;
}
