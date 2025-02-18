fn sin_68d3ab() {
  const arg_0 = vec2(1.57079632679000003037);
  var res = sin(arg_0);
}

@fragment
fn fragment_main() {
  sin_68d3ab();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_68d3ab();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  sin_68d3ab();
  return out;
}
