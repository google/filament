var<private> t : bool;
fn m() -> bool {
    t = true;
    return bool(t);
}
fn f() {
    var v : f32 = f32(m());
}