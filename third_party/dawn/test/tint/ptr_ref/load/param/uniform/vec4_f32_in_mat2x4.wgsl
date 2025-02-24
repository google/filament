@group(0) @binding(0) var<uniform> S : mat2x4<f32>;

fn func(pointer : ptr<uniform, vec4<f32>>) -> vec4<f32> {
  return *pointer;
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&S[1]);
}
