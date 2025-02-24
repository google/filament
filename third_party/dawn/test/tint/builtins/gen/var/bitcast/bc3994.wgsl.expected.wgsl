enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn bitcast_bc3994() -> vec4<f16> {
  var arg_0 = vec2<u32>(1u);
  var res : vec4<f16> = bitcast<vec4<f16>>(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = bitcast_bc3994();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = bitcast_bc3994();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<f16>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = bitcast_bc3994();
  return out;
}
