fn vector() {
    let idx = 3;
    let x = vec2(1,2)[idx];
}

fn matrix() {
    let idx = 4;
    let x = mat2x2(1,2,3,4)[idx];
}

fn fixed_size_array() {
    let arr = array(1,2);
    let idx = 3;
    let x = arr[idx];
}

@group(0) @binding(0) var<storage> rarr : array<f32>;
fn runtime_size_array() {
    let idx = -1;
    let x = rarr[idx];
}

@compute @workgroup_size(1)
fn f() {
    vector();
    matrix();
    fixed_size_array();
    runtime_size_array();
}
