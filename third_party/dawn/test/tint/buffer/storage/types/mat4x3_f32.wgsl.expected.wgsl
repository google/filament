@group(0) @binding(0) var<storage, read> in : mat4x3<f32>;

@group(0) @binding(1) var<storage, read_write> out : mat4x3<f32>;

@compute @workgroup_size(1)
fn main() {
  out = in;
}
