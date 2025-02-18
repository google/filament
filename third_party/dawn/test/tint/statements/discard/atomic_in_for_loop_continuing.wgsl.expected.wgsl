@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

@group(0) @binding(2) var<storage, read_write> a : atomic<i32>;

@fragment
fn foo(@location(0) in : f32, @location(1) coord : vec2<f32>) -> @location(0) i32 {
  if ((in == 0.0)) {
    discard;
  }
  var result = i32(textureSample(t, s, coord).x);
  for(var i = 0; (i < 10); i = atomicAdd(&(a), 1)) {
    result += i;
  }
  return result;
}
