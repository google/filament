struct S {
  arr : array<i32>,
}

@binding(0) @group(0) var<storage, read_write> s : S;

@compute @workgroup_size(1)
fn main() {
  _ = &(s.arr);
}
