@group(0) @binding(0)
var<storage, read> in : i32;

@group(0) @binding(1)
var<storage, read_write> out : i32;

@compute @workgroup_size(1)
fn main() {
  out = in;
}
