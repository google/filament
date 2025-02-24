@group(1) @binding(0) var arg_0 : texture_storage_2d<rg32float, write>;

fn textureStore_064c7f() {
  textureStore(arg_0, vec2<i32>(1i), vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_064c7f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_064c7f();
}
