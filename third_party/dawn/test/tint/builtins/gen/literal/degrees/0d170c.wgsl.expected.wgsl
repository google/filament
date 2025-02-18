@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn degrees_0d170c() -> vec4<f32> {
  var res : vec4<f32> = degrees(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = degrees_0d170c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = degrees_0d170c();
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
  out.prevent_dce = degrees_0d170c();
  return out;
}
