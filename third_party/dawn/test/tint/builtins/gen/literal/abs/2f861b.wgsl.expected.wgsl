fn abs_2f861b() {
  var res = abs(vec3(1.0));
}

@fragment
fn fragment_main() {
  abs_2f861b();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_2f861b();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  abs_2f861b();
  return out;
}
