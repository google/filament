enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn saturate_cd2028() -> vec2<f16> {
  var res : vec2<f16> = saturate(vec2<f16>(2.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = saturate_cd2028();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = saturate_cd2028();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<f16>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = saturate_cd2028();
  return out;
}
