@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn determinant_a0a87c() -> f32 {
  var res : f32 = determinant(mat4x4<f32>(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = determinant_a0a87c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = determinant_a0a87c();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = determinant_a0a87c();
  return out;
}
