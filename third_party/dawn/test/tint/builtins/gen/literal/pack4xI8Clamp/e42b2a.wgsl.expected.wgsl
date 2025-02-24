@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn pack4xI8Clamp_e42b2a() -> u32 {
  var res : u32 = pack4xI8Clamp(vec4<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = pack4xI8Clamp_e42b2a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = pack4xI8Clamp_e42b2a();
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
  out.prevent_dce = pack4xI8Clamp_e42b2a();
  return out;
}
