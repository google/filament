var<workgroup> S : mat2x2<f32>;

fn func(pointer : ptr<workgroup, vec2<f32>>) -> vec2<f32> {
  return *(pointer);
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&(S[1]));
}
