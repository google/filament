fn v() -> vec3<f32> {
    return vec3(0.0);
}

fn f() {
    let a = vec3(1.0);
    let b = vec3(a);
    let c = vec3(v());
    let d = vec3(a * 2);
}
