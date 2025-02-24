@group(0) @binding(0) var<storage, read_write> s: i32;

@compute @workgroup_size(1)
fn f() {
    let a = 1;
    let _a = a;
    let b = a;
    let _b = _a;

    s = a + _a + b + _b;
}
