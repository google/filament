fn transpose_7eb2c5() {
  var res = transpose(mat2x2(1.0, 1.0, 1.0, 1.0));
}

@fragment
fn fragment_main() {
  transpose_7eb2c5();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_7eb2c5();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  transpose_7eb2c5();
  return out;
}
