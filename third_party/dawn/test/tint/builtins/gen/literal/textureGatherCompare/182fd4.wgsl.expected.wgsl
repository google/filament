@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@group(1) @binding(0) var arg_0 : texture_depth_cube;

@group(1) @binding(1) var arg_1 : sampler_comparison;

fn textureGatherCompare_182fd4() -> vec4<f32> {
  var res : vec4<f32> = textureGatherCompare(arg_0, arg_1, vec3<f32>(1.0f), 1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureGatherCompare_182fd4();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureGatherCompare_182fd4();
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
  out.prevent_dce = textureGatherCompare_182fd4();
  return out;
}
