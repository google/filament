@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba16uint, write>;

fn textureStore_fd350c() {
  textureStore(arg_0, vec2<i32>(1i), 1u, vec4<u32>(1u));
}

@fragment
fn fragment_main() {
  textureStore_fd350c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_fd350c();
}
