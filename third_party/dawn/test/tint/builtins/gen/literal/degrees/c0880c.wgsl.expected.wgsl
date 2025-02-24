fn degrees_c0880c() {
  var res = degrees(vec3(1.0));
}

@fragment
fn fragment_main() {
  degrees_c0880c();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_c0880c();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  degrees_c0880c();
  return out;
}
