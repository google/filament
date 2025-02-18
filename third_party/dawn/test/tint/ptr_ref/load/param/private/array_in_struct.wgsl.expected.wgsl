struct str {
  arr : array<i32, 4>,
}

fn func(pointer : ptr<private, array<i32, 4>>) -> array<i32, 4> {
  return *(pointer);
}

var<private> P : str;

@compute @workgroup_size(1)
fn main() {
  let r = func(&(P.arr));
}
