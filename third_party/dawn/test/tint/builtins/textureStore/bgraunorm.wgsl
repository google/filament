@group(0) @binding(0) var tex: texture_storage_2d<bgra8unorm, write>;

@fragment
fn fragment_main() {
  let value = vec4(1f, 2f, 3f, 4f);
  textureStore(tex, vec2(9, 8), value);
}
