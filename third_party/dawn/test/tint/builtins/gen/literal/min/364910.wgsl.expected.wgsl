fn min_364910() {
  var res = min(vec3(1.0), vec3(1.0));
}

@fragment
fn fragment_main() {
  min_364910();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_364910();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  min_364910();
  return out;
}
