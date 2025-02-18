struct str {
  i : i32,
};

fn func(pointer : ptr<function, i32>) -> i32 {
  return *pointer;
}

@compute @workgroup_size(1)
fn main() {
  var F : str;
  let r = func(&F.i);
}
