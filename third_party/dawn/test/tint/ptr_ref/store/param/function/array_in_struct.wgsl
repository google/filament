struct str {
  arr : array<i32, 4>,
};

fn func(pointer : ptr<function, array<i32, 4>>) {
  *pointer = array<i32, 4>();
}

@compute @workgroup_size(1)
fn main() {
  var F : str;
  func(&F.arr);
}
