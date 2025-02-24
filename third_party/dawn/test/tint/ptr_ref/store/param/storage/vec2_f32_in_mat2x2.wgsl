@group(0) @binding(0) var<storage, read_write> S : mat2x2<f32>;

fn func(pointer : ptr<storage, vec2<f32>, read_write>) {
  *pointer = vec2<f32>();
}

@compute @workgroup_size(1)
fn main() {
  func(&S[1]);
}
