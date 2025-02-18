fn func(pointer : ptr<function, vec4<f32>>) -> vec4<f32> {
  return *pointer;
}

@compute @workgroup_size(1)
fn main() {
  var F : vec4<f32>;
  let r = func(&F);
}
