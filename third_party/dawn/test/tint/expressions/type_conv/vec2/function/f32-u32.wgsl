var<private> t : f32;
fn m() -> vec2<f32> {
    t = 1.0f;
    return vec2<f32>(t);
}
fn f() {
    var v : vec2<u32> = vec2<u32>(m());
}