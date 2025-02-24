@compute @workgroup_size(1)
fn f() {
    var a = vec3<u32>(1u, 2u, 3u);
    var b = 0u;
    let r : vec3<u32> = a / b;
}
