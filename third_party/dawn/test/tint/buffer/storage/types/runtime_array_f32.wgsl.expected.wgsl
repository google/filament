@group(0) @binding(0) var<storage, read> in : array<f32>;

@group(0) @binding(1) var<storage, read_write> out : array<f32>;

@compute @workgroup_size(1)
fn main() {
  out[0] = in[0];
}
