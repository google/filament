fn dot_eb9fbf() {
  const arg_0 = vec4(1);
  const arg_1 = vec4(1);
  var res = dot(arg_0, arg_1);
}

@fragment
fn fragment_main() {
  dot_eb9fbf();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_eb9fbf();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  dot_eb9fbf();
  return out;
}
