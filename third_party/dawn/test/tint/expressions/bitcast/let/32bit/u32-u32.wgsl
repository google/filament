@compute @workgroup_size(1)
fn f() {
    let a : u32 = u32(1073757184u);
    let b : u32 = bitcast<u32>(a);
}
