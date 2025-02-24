fn refract_8c192a() {
  var res = refract(vec4(1.0), vec4(1.0), 1.0);
}

@fragment
fn fragment_main() {
  refract_8c192a();
}

@compute @workgroup_size(1)
fn compute_main() {
  refract_8c192a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  refract_8c192a();
  return out;
}
