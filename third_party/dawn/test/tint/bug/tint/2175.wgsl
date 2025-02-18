// flags: --rename-all
//
const v = vec4();
const x = v.x * 2u;

@group(0) @binding(0)
var<storage, read_write> output : u32;

@compute @workgroup_size(1)
fn main() {
  output = x;
}
