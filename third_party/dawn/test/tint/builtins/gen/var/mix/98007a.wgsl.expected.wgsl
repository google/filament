fn mix_98007a() {
  const arg_0 = vec4(1.0);
  const arg_1 = vec4(1.0);
  const arg_2 = vec4(1.0);
  var res = mix(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  mix_98007a();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_98007a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  mix_98007a();
  return out;
}
