fn tanh_313aa1() {
  var res = tanh(1.0);
}

@fragment
fn fragment_main() {
  tanh_313aa1();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_313aa1();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  tanh_313aa1();
  return out;
}
