var<private> v : vec3<i32>;

@compute @workgroup_size(1)
fn main() {
    _ = v;
}
