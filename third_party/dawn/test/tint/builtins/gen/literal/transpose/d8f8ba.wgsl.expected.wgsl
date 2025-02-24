@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn transpose_d8f8ba() -> i32 {
  var res : mat4x3<f32> = transpose(mat3x4<f32>(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));
  return select(0, 1, (res[0][0] == 0));
}

@fragment
fn fragment_main() {
  prevent_dce = transpose_d8f8ba();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = transpose_d8f8ba();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : i32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = transpose_d8f8ba();
  return out;
}
