@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn bitcast_dce842() -> u32 {
  var arg_0 = 1i;
  var res : u32 = bitcast<u32>(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = bitcast_dce842();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = bitcast_dce842();
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
  out.prevent_dce = bitcast_dce842();
  return out;
}
