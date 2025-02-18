@compute @workgroup_size(1)
fn f() {
    var a = vec3<u32>(1u, 2u, 3u);
    var b = vec3<u32>(0u, 5u, 0u);
    let r : vec3<u32> = a % b;
}
