struct Constants {
  zero : u32,
}

@group(1) @binding(0) var<uniform> constants : Constants;

struct Result {
  value : u32,
}

@group(1) @binding(1) var<storage, read_write> result : Result;

struct S {
  data : array<u32, 3>,
}

var<private> s : S;

@compute @workgroup_size(1)
fn main() {
  s.data[constants.zero] = 0u;
}
