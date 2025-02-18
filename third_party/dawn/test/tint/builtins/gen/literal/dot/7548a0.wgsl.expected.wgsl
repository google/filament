@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn dot_7548a0() -> u32 {
  var res : u32 = dot(vec3<u32>(1u), vec3<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dot_7548a0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = dot_7548a0();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : u32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = dot_7548a0();
  return out;
}
