@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba8uint, write>;

fn textureStore_77c0ae() {
  textureStore(arg_0, vec2<i32>(1i), vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_77c0ae();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_77c0ae();
}
