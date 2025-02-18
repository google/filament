@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn sin_01f241() -> vec3<f32> {
  var res : vec3<f32> = sin(vec3<f32>(1.57079637050628662109f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = sin_01f241();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = sin_01f241();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = sin_01f241();
  return out;
}
