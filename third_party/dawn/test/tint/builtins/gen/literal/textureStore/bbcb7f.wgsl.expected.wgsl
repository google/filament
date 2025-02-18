@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba8sint, write>;

fn textureStore_bbcb7f() {
  textureStore(arg_0, vec2<i32>(1i), vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_bbcb7f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_bbcb7f();
}
