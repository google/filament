struct str {
  arr : array<i32, 4>,
}

@group(0) @binding(0) var<storage> S : str;

fn func(pointer : ptr<storage, array<i32, 4>>) -> array<i32, 4> {
  return *(pointer);
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&(S.arr));
}
