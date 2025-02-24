@group(0) @binding(0) var<storage, read> in : vec4<i32>;

@group(0) @binding(1) var<storage, read_write> out : vec4<i32>;

@compute @workgroup_size(1)
fn main() {
  out = in;
}
