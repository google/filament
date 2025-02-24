struct str {
  arr : array<i32, 4>,
};

@group(0) @binding(0) var<storage, read_write> S : str;

fn func(pointer : ptr<storage, array<i32, 4>, read_write>) {
  *pointer = array<i32, 4>();
}

@compute @workgroup_size(1)
fn main() {
  func(&S.arr);
}
