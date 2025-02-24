@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn pack4xU8_b70b53() -> u32 {
  var arg_0 = vec4<u32>(1u);
  var res : u32 = pack4xU8(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = pack4xU8_b70b53();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = pack4xU8_b70b53();
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
  out.prevent_dce = pack4xU8_b70b53();
  return out;
}
