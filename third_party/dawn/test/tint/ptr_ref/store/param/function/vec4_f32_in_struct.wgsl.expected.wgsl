struct str {
  i : vec4<f32>,
}

fn func(pointer : ptr<function, vec4<f32>>) {
  *(pointer) = vec4<f32>();
}

@compute @workgroup_size(1)
fn main() {
  var F : str;
  func(&(F.i));
}
