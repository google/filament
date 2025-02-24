struct str {
  i : vec4<f32>,
};

@group(0) @binding(0) var<storage, read_write> S : str;

fn func(pointer : ptr<storage, vec4<f32>, read_write>) {
  *pointer = vec4<f32>();
}

@compute @workgroup_size(1)
fn main() {
  func(&S.i);
}
