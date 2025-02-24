fn func(pointer : ptr<private, vec2<f32>>) -> vec2<f32> {
  return *pointer;
}

var<private> P : mat2x2<f32>;

@compute @workgroup_size(1)
fn main() {
  let r = func(&P[1]);
}
