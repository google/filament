fn func(pointer : ptr<function, vec2<f32>>) -> vec2<f32> {
  return *(pointer);
}

@compute @workgroup_size(1)
fn main() {
  var F : mat2x2<f32>;
  let r = func(&(F[1]));
}
