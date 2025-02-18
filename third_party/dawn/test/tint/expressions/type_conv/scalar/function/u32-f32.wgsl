var<private> t : u32;
fn m() -> u32 {
    t = 1u;
    return u32(t);
}
fn f() {
    var v : f32 = f32(m());
}