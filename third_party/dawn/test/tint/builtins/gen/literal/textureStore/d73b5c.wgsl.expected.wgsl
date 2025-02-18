@group(1) @binding(0) var arg_0 : texture_storage_1d<rg32sint, write>;

fn textureStore_d73b5c() {
  textureStore(arg_0, 1i, vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  textureStore_d73b5c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_d73b5c();
}
