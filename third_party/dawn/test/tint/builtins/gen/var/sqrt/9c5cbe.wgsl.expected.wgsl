fn sqrt_9c5cbe() {
  const arg_0 = vec2(1.0);
  var res = sqrt(arg_0);
}

@fragment
fn fragment_main() {
  sqrt_9c5cbe();
}

@compute @workgroup_size(1)
fn compute_main() {
  sqrt_9c5cbe();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  sqrt_9c5cbe();
  return out;
}
