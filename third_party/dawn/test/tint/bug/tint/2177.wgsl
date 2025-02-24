@binding(0) @group(0) var<storage, read_write> arr : array<u32>;

fn f2(p : ptr<storage, array<u32>, read_write>) -> u32 {
  return arrayLength(p);
}

fn f1(p : ptr<storage, array<u32>, read_write>) -> u32 {
  return f2(p);
}

fn f0(p : ptr<storage, array<u32>, read_write>) -> u32 {
  return f1(p);
}

@compute @workgroup_size(1)
fn main() {
  arr[0] = f0(&arr);
}
