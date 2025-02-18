fn inverseSqrt_6d0783() {
  var res = inverseSqrt(vec3(1.0));
}

@fragment
fn fragment_main() {
  inverseSqrt_6d0783();
}

@compute @workgroup_size(1)
fn compute_main() {
  inverseSqrt_6d0783();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  inverseSqrt_6d0783();
  return out;
}
