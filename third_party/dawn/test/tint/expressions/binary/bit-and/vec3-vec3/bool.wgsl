@compute @workgroup_size(1)
fn f() {
    let a = vec3<bool>(true, true, false);
    let b = vec3<bool>(true, false, true);
    let r : vec3<bool> = a & b;
}
