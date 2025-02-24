@group(0) @binding(0) var t_f : texture_1d<f32>;

@group(0) @binding(1) var t_i : texture_1d<i32>;

@group(0) @binding(2) var t_u : texture_1d<u32>;

@compute @workgroup_size(1)
fn main() {
  var fdims = textureDimensions(t_f, 1);
  var idims = textureDimensions(t_i, 1);
  var udims = textureDimensions(t_u, 1);
}
