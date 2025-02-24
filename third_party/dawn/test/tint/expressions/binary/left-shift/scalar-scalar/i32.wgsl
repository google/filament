@compute @workgroup_size(1)
fn f() {
    let a = 1;
    let b = 2u;
    let r : i32 = a << b;
}
