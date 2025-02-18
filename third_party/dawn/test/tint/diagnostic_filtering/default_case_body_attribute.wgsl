@group(0) @binding(1) var t : texture_2d<f32>;
@group(0) @binding(2) var s : sampler;

@fragment
fn main(@location(0) x : f32) {
  switch (i32(x)) {
    default @diagnostic(warning, derivative_uniformity) {
      _ = textureSample(t, s, vec2(0, 0));
    }
  }
}
