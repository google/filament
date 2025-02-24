fn func(pointer : ptr<private, vec4<f32>>) -> vec4<f32> {
  return *(pointer);
}

var<private> P : vec4<f32>;

@compute @workgroup_size(1)
fn main() {
  let r = func(&(P));
}
