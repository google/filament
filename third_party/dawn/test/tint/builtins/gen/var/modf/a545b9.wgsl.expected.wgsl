enable f16;

fn modf_a545b9() {
  var arg_0 = vec2<f16>(-(1.5h));
  var res = modf(arg_0);
}

@fragment
fn fragment_main() {
  modf_a545b9();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_a545b9();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  modf_a545b9();
  return out;
}
