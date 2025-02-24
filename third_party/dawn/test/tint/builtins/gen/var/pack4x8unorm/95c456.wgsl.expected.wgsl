@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn pack4x8unorm_95c456() -> u32 {
  var arg_0 = vec4<f32>(1.0f);
  var res : u32 = pack4x8unorm(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = pack4x8unorm_95c456();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = pack4x8unorm_95c456();
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
  out.prevent_dce = pack4x8unorm_95c456();
  return out;
}
