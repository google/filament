@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rg32float, write>;

fn textureStore_658a74() {
  textureStore(arg_0, vec2<i32>(1i), 1u, vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_658a74();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_658a74();
}
