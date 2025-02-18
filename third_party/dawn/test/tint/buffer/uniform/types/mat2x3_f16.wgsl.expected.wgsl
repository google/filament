enable f16;

@group(0) @binding(0) var<uniform> u : mat2x3<f16>;

@group(0) @binding(1) var<storage, read_write> s : mat2x3<f16>;

@compute @workgroup_size(1)
fn main() {
  let x = u;
  s = x;
}
