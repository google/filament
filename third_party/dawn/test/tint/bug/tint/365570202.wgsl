// flags: --glsl-desktop

@group(0) @binding(0)
var tex : texture_storage_2d<rgba8unorm, read_write>;

@compute @workgroup_size(1)
fn main() {
  textureStore(tex, vec2(), vec4());
}
