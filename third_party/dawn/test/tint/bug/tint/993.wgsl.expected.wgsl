struct Constants {
  zero : u32,
}

@group(1) @binding(0) var<uniform> constants : Constants;

struct Result {
  value : u32,
}

@group(1) @binding(1) var<storage, read_write> result : Result;

struct TestData {
  data : array<atomic<i32>, 3>,
}

@group(0) @binding(0) var<storage, read_write> s : TestData;

fn runTest() -> i32 {
  return atomicLoad(&(s.data[(0u + u32(constants.zero))]));
}

@compute @workgroup_size(1)
fn main() {
  result.value = u32(runTest());
}
