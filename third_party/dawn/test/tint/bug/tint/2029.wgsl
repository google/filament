@group(0) @binding(0) var<storage, read_write> s : vec3<i32>;
@compute @workgroup_size(1)
fn main() {
  s = vec3i(1, 1, 1);
}
