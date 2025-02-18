struct str {
  i : vec4<f32>,
}

var<workgroup> S : str;

fn func(pointer : ptr<workgroup, vec4<f32>>) -> vec4<f32> {
  return *(pointer);
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&(S.i));
}
