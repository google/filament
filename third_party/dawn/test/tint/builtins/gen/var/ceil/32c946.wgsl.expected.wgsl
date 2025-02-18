fn ceil_32c946() {
  const arg_0 = vec3(1.5);
  var res = ceil(arg_0);
}

@fragment
fn fragment_main() {
  ceil_32c946();
}

@compute @workgroup_size(1)
fn compute_main() {
  ceil_32c946();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  ceil_32c946();
  return out;
}
