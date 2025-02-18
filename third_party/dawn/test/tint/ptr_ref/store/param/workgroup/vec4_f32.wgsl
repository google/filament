var<workgroup> S : vec4<f32>;

fn func(pointer : ptr<workgroup, vec4<f32>>) {
  *pointer = vec4<f32>();
}

@compute @workgroup_size(1)
fn main() {
  func(&S);
}
