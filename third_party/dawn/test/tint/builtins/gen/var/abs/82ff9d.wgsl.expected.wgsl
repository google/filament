fn abs_82ff9d() {
  const arg_0 = vec2(1.0);
  var res = abs(arg_0);
}

@fragment
fn fragment_main() {
  abs_82ff9d();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_82ff9d();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  abs_82ff9d();
  return out;
}
