fn refract_d7569b() {
  var res = refract(vec3(1.0), vec3(1.0), 1.0);
}

@fragment
fn fragment_main() {
  refract_d7569b();
}

@compute @workgroup_size(1)
fn compute_main() {
  refract_d7569b();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  refract_d7569b();
  return out;
}
