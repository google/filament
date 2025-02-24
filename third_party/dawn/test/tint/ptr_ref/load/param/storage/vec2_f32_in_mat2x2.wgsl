@group(0) @binding(0) var<storage> S : mat2x2<f32>;

fn func(pointer : ptr<storage, vec2<f32>>) -> vec2<f32> {
  return *pointer;
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&S[1]);
}
