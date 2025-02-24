fn length_936ad5() {
  const arg_0 = 0.0;
  var res = length(arg_0);
}

@fragment
fn fragment_main() {
  length_936ad5();
}

@compute @workgroup_size(1)
fn compute_main() {
  length_936ad5();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  length_936ad5();
  return out;
}
