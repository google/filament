@group(0) @binding(0) var tex : texture_storage_2d<bgra8unorm, write>;

@fragment
fn fragment_main() {
  let value = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  textureStore(tex, vec2(9, 8), value);
}
