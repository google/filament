struct str {
  arr : array<i32, 4>,
}

var<workgroup> S : str;

fn func(pointer : ptr<workgroup, array<i32, 4>>) -> array<i32, 4> {
  return *(pointer);
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&(S.arr));
}
