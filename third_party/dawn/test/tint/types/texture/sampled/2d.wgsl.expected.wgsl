@group(0) @binding(0) var t_f : texture_2d<f32>;

@group(0) @binding(1) var t_i : texture_2d<i32>;

@group(0) @binding(2) var t_u : texture_2d<u32>;

@compute @workgroup_size(1)
fn main() {
  var fdims = textureDimensions(t_f, 1);
  var idims = textureDimensions(t_i, 1);
  var udims = textureDimensions(t_u, 1);
}
