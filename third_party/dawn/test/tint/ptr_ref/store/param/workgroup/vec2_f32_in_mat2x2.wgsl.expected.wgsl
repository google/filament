var<workgroup> S : mat2x2<f32>;

fn func(pointer : ptr<workgroup, vec2<f32>>) {
  *(pointer) = vec2<f32>();
}

@compute @workgroup_size(1)
fn main() {
  func(&(S[1]));
}
