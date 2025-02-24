fn select_dfab3b() {
  const arg_0 = vec3(1);
  const arg_1 = vec3(1);
  var arg_2 = true;
  var res = select(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  select_dfab3b();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_dfab3b();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  select_dfab3b();
  return out;
}
