@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@group(1) @binding(0) var arg_0 : texture_3d<f32>;

fn textureDimensions_0890c6() -> vec3<u32> {
  var arg_1 = 1u;
  var res : vec3<u32> = textureDimensions(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_0890c6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_0890c6();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<u32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureDimensions_0890c6();
  return out;
}
