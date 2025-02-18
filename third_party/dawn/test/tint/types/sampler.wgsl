@group(1) @binding(0) var t : texture_2d<f32>;
@group(1) @binding(1) var d : texture_depth_2d;

@group(0) @binding(0) var s : sampler;
@group(0) @binding(1) var sc : sampler_comparison;

@fragment
fn main() {
  var a = textureSample(t, s, vec2(1, 1));
  var b = textureGatherCompare(d, sc, vec2(1, 1), 1);
}
