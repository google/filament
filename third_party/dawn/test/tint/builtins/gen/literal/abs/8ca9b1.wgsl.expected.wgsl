fn abs_8ca9b1() {
  var res = abs(vec4(1));
}

@fragment
fn fragment_main() {
  abs_8ca9b1();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_8ca9b1();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  abs_8ca9b1();
  return out;
}
