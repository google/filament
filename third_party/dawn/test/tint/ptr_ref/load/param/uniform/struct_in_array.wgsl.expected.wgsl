struct str {
  i : vec4<i32>,
}

@group(0) @binding(0) var<uniform> S : array<str, 4>;

fn func(pointer : ptr<uniform, str>) -> str {
  return *(pointer);
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&(S[2]));
}
