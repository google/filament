@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

@group(1) @binding(0) var arg_0 : texture_2d<i32>;

fn textureDimensions_2e443d() -> vec2<u32> {
  var arg_1 = 1i;
  var res : vec2<u32> = textureDimensions(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_2e443d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_2e443d();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<u32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureDimensions_2e443d();
  return out;
}
