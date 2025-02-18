fn dot_c11efe() {
  var res = dot(vec3(1), vec3(1));
}

@fragment
fn fragment_main() {
  dot_c11efe();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_c11efe();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  dot_c11efe();
  return out;
}
