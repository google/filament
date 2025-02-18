@group(0) @binding(0) var<storage> S : vec4<f32>;

fn func(pointer : ptr<storage, vec4<f32>>) -> vec4<f32> {
  return *(pointer);
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&(S));
}
