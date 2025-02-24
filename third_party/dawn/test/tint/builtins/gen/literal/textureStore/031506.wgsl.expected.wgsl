@group(1) @binding(0) var arg_0 : texture_storage_2d<r32float, read_write>;

fn textureStore_031506() {
  textureStore(arg_0, vec2<i32>(1i), vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  textureStore_031506();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_031506();
}
