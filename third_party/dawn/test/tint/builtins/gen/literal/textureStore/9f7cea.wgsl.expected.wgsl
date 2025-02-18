@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba8uint, write>;

fn textureStore_9f7cea() {
  textureStore(arg_0, vec2<i32>(1i), 1u, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_9f7cea();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_9f7cea();
}
