struct str {
  arr : array<i32, 4>,
};

fn func(pointer : ptr<function, array<i32, 4>>) -> array<i32, 4> {
  return *pointer;
}

@compute @workgroup_size(1)
fn main() {
  var F : str;
  let r = func(&F.arr);
}
