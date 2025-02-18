struct str {
  i : i32,
};

@group(0) @binding(0) var<storage, read_write> S : array<str, 4>;

fn func(pointer : ptr<storage, str, read_write>) {
  *pointer = str();
}

@compute @workgroup_size(1)
fn main() {
  func(&S[2]);
}
