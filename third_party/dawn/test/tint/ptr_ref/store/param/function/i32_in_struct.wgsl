struct str {
  i : i32,
};

fn func(pointer : ptr<function, i32>) {
  *pointer = 42;
}

@compute @workgroup_size(1)
fn main() {
  var F : str;
  func(&F.i);
}
