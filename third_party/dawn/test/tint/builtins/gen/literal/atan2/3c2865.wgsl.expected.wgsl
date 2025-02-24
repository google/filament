fn atan2_3c2865() {
  var res = atan2(vec3(1.0), vec3(1.0));
}

@fragment
fn fragment_main() {
  atan2_3c2865();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan2_3c2865();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  atan2_3c2865();
  return out;
}
