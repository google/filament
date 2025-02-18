fn abs_577d6e() {
  var res = abs(vec2(1));
}

@fragment
fn fragment_main() {
  abs_577d6e();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_577d6e();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  abs_577d6e();
  return out;
}
