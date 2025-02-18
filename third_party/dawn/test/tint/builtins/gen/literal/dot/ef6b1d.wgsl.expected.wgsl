@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn dot_ef6b1d() -> i32 {
  var res : i32 = dot(vec4<i32>(1i), vec4<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dot_ef6b1d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = dot_ef6b1d();
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
  out.prevent_dce = dot_ef6b1d();
  return out;
}
