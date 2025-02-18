@compute @workgroup_size(1)
fn f() {
    let a = 4u;
    let b = vec3<u32>(0u, 2u, 0u);
    let r : vec3<u32> = a / b;
}
