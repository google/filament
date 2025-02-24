@compute @workgroup_size(1)
fn f() {
    let a = 1u;
    let b = 2u;
    let r : u32 = a / b;
}
