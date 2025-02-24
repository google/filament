fn asinh_cf8603() {
  const arg_0 = vec4(1.0);
  var res = asinh(arg_0);
}

@fragment
fn fragment_main() {
  asinh_cf8603();
}

@compute @workgroup_size(1)
fn compute_main() {
  asinh_cf8603();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  asinh_cf8603();
  return out;
}
