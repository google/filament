fn func(pointer : ptr<private, vec4<f32>>) {
  *(pointer) = vec4<f32>();
}

var<private> P : mat2x4<f32>;

@compute @workgroup_size(1)
fn main() {
  func(&(P[1]));
}
