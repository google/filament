fn sqrt_8da177() {
  var res = sqrt(1.0);
}

@fragment
fn fragment_main() {
  sqrt_8da177();
}

@compute @workgroup_size(1)
fn compute_main() {
  sqrt_8da177();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  sqrt_8da177();
  return out;
}
