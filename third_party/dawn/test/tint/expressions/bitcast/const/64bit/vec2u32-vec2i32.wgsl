@compute @workgroup_size(1)
fn f() {
    const a : vec2<u32> = vec2<u32>(1073757184u, 3288351232u);
    let b : vec2<i32> = bitcast<vec2<i32>>(a);
}
