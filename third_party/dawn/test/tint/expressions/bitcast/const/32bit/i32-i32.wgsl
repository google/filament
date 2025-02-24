@compute @workgroup_size(1)
fn f() {
    const a : i32 = i32(1073757184i);
    let b : i32 = bitcast<i32>(a);
}
