fn c(z: i32) -> i32 {
    var a = 1 + z;
    a = a + 2;
    return a;
}

fn b() {
    var b = c(2);
    b += c(3);
}
