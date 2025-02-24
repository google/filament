@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn firstLeadingBit_c1f940() -> vec4<i32> {
  var res : vec4<i32> = firstLeadingBit(vec4<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = firstLeadingBit_c1f940();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = firstLeadingBit_c1f940();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<i32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = firstLeadingBit_c1f940();
  return out;
}
