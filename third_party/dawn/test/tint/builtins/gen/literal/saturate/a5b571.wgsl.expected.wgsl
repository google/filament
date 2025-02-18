@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn saturate_a5b571() -> vec4<f32> {
  var res : vec4<f32> = saturate(vec4<f32>(2.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = saturate_a5b571();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = saturate_a5b571();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = saturate_a5b571();
  return out;
}
