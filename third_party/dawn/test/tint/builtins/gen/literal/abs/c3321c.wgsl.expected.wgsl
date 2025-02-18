fn abs_c3321c() {
  var res = abs(vec3(1));
}

@fragment
fn fragment_main() {
  abs_c3321c();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_c3321c();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  abs_c3321c();
  return out;
}
