@group(0) @binding(0) var<uniform> u : vec3<u32>;

@group(0) @binding(1) var<storage, read_write> s : vec3<u32>;

@compute @workgroup_size(1)
fn main() {
  let x = u;
  s = x;
}
