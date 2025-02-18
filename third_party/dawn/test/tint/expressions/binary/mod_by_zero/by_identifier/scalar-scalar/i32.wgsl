@compute @workgroup_size(1)
fn f() {
    var a = 1;
    var b = 0;
    let r : i32 = a % b;
}
