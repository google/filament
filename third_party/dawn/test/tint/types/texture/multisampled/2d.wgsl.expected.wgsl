@group(0) @binding(0) var t_f : texture_multisampled_2d<f32>;

@group(0) @binding(1) var t_i : texture_multisampled_2d<i32>;

@group(0) @binding(2) var t_u : texture_multisampled_2d<u32>;

@compute @workgroup_size(1)
fn main() {
  var fdims = textureDimensions(t_f);
  var idims = textureDimensions(t_i);
  var udims = textureDimensions(t_u);
}
