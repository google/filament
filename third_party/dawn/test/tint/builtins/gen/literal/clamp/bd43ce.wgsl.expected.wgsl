@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn clamp_bd43ce() -> vec4<u32> {
  var res : vec4<u32> = clamp(vec4<u32>(1u), vec4<u32>(1u), vec4<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = clamp_bd43ce();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = clamp_bd43ce();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<u32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = clamp_bd43ce();
  return out;
}
