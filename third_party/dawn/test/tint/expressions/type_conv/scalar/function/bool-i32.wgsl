var<private> t : bool;
fn m() -> bool {
    t = true;
    return bool(t);
}
fn f() {
    var v : i32 = i32(m());
}