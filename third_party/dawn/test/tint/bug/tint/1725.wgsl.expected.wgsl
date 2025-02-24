@group(0) @binding(0) var<storage> data : array<u32>;

@compute @workgroup_size(1)
fn main(@builtin(local_invocation_index) local_invocation_index : u32) {
  let min = 0;
  let max = 0;
  let arrayLength = 0;
  let x = data[local_invocation_index];
}
