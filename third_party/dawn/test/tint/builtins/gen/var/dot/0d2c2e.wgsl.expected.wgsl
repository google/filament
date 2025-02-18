fn dot_0d2c2e() {
  const arg_0 = vec2(1.0);
  const arg_1 = vec2(1.0);
  var res = dot(arg_0, arg_1);
}

@fragment
fn fragment_main() {
  dot_0d2c2e();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_0d2c2e();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  dot_0d2c2e();
  return out;
}
