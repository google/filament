enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

struct SB_RO {
  arg_0 : array<f16>,
}

@group(0) @binding(1) var<storage, read> sb_ro : SB_RO;

fn arrayLength_9ea96d() -> u32 {
  var res : u32 = arrayLength(&(sb_ro.arg_0));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = arrayLength_9ea96d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = arrayLength_9ea96d();
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
  out.prevent_dce = arrayLength_9ea96d();
  return out;
}
