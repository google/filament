@group(0) @binding(0) var<storage, read_write> S : mat2x4<f32>;

fn func(pointer : ptr<storage, vec4<f32>, read_write>) {
  *pointer = vec4<f32>();
}

@compute @workgroup_size(1)
fn main() {
  func(&S[1]);
}
