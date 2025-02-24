fn abs_e28785() {
  var res = abs(vec4(1.0));
}

@fragment
fn fragment_main() {
  abs_e28785();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_e28785();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  abs_e28785();
  return out;
}
