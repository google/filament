fn atanh_c5dc32() {
  var res = atanh(0.5);
}

@fragment
fn fragment_main() {
  atanh_c5dc32();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_c5dc32();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  atanh_c5dc32();
  return out;
}
