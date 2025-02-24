fn func(pointer : ptr<private, vec2<f32>>) {
  *(pointer) = vec2<f32>();
}

var<private> P : mat2x2<f32>;

@compute @workgroup_size(1)
fn main() {
  func(&(P[1]));
}
