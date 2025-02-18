enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

fn log2_fb9f0b() -> vec2<f16> {
  var arg_0 = vec2<f16>(1.0h);
  var res : vec2<f16> = log2(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = log2_fb9f0b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = log2_fb9f0b();
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
  out.prevent_dce = log2_fb9f0b();
  return out;
}
