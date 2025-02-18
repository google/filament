@compute @workgroup_size(1)
fn f() {
    let a = vec3<u32>(1u, 2u, 3u);
    let b = 4u;
    let r : vec3<u32> = a + b;
}
