@group(0) @binding(0) var<uniform> u : vec2<i32>;
@group(0) @binding(1) var<storage, read_write> s: vec2<i32>;

@compute @workgroup_size(1)
fn main() {
  let x = u;

  s = x;
}
