struct str {
  arr : array<vec4<i32>, 4>,
};

@group(0) @binding(0) var<uniform> S : str;

fn func(pointer : ptr<uniform, array<vec4<i32>, 4>>) -> array<vec4<i32>, 4> {
  return *pointer;
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&S.arr);
}
