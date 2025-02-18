@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn bitcast_e6c18f() -> u32 {
  var res : u32 = bitcast<u32>(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = bitcast_e6c18f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = bitcast_e6c18f();
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
  out.prevent_dce = bitcast_e6c18f();
  return out;
}
