fn tanh_ac5d33() {
  var res = tanh(vec4(1.0));
}

@fragment
fn fragment_main() {
  tanh_ac5d33();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_ac5d33();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  tanh_ac5d33();
  return out;
}
