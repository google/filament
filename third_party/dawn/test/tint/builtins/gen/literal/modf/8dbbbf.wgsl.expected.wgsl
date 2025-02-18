enable f16;

fn modf_8dbbbf() {
  var res = modf(-(1.5h));
}

@fragment
fn fragment_main() {
  modf_8dbbbf();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_8dbbbf();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  modf_8dbbbf();
  return out;
}
