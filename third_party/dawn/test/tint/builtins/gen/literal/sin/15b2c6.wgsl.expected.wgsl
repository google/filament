fn sin_15b2c6() {
  var res = sin(vec4(1.57079632679000003037));
}

@fragment
fn fragment_main() {
  sin_15b2c6();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_15b2c6();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  sin_15b2c6();
  return out;
}
