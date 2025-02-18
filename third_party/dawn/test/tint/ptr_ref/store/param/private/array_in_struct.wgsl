struct str {
  arr : array<i32, 4>,
};

fn func(pointer : ptr<private, array<i32, 4>>) {
  *pointer = array<i32, 4>();
}

var<private> P : str;

@compute @workgroup_size(1)
fn main() {
  func(&P.arr);
}
