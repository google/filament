@group(0) @binding(0) var<storage, read> in : array<f32, 4>;

@group(0) @binding(1) var<storage, read_write> out : array<f32, 4>;

@compute @workgroup_size(1)
fn main() {
  out = in;
}
