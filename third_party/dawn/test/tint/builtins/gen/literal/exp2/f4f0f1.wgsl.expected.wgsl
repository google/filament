fn exp2_f4f0f1() {
  var res = exp2(1.0);
}

@fragment
fn fragment_main() {
  exp2_f4f0f1();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_f4f0f1();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  exp2_f4f0f1();
  return out;
}
