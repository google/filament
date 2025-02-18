@group(1) @binding(0) var t : texture_2d<f32>;

@group(1) @binding(1) var s : sampler;

@fragment
fn main() {
  var res : vec4<f32> = textureGather(0, t, s, vec2<f32>());
}
