@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba16sint, write>;

fn textureStore_3a52ac() {
  textureStore(arg_0, vec2<i32>(1i), 1i, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_3a52ac();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_3a52ac();
}
