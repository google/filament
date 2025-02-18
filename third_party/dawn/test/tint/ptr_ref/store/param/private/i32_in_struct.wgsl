struct str {
  i : i32,
};

fn func(pointer : ptr<private, i32>) {
  *pointer = 42;
}

var<private> P : str;

@compute @workgroup_size(1)
fn main() {
  func(&P.i);
}
