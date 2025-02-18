@binding(0) @group(0) var t : texture_2d<f32>;

@binding(0) @group(1) var s : sampler;

@fragment
fn f() -> @location(0) vec4<f32> {
  return textureSample(t, s, vec2<f32>(), (vec2<i32>(1, 2) + vec2<i32>(3, 4)));
}
