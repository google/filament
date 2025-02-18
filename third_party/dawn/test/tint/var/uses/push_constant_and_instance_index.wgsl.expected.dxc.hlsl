SKIP: INVALID


enable chromium_experimental_push_constant;

var<push_constant> a : f32;

@vertex
fn main(@builtin(instance_index) b : u32) -> @builtin(position) vec4<f32> {
  return vec4<f32>((a + f32(b)));
}

Failed to generate: error: unhandled address space push_constant

tint executable returned error: exit status 1
