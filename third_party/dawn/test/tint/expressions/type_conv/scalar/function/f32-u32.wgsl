var<private> t : f32;
fn m() -> f32 {
    t = 1.0f;
    return f32(t);
}
fn f() {
    var v : u32 = u32(m());
}