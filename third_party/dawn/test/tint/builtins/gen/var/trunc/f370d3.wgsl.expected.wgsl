@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn trunc_f370d3() -> vec2<f32> {
  var arg_0 = vec2<f32>(1.5f);
  var res : vec2<f32> = trunc(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = trunc_f370d3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = trunc_f370d3();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = trunc_f370d3();
  return out;
}
