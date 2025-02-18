fn modf_68d8ee() {
  const arg_0 = vec3(-(1.5));
  var res = modf(arg_0);
}

@fragment
fn fragment_main() {
  modf_68d8ee();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_68d8ee();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  modf_68d8ee();
  return out;
}
