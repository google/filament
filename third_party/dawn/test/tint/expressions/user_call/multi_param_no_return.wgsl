fn c(x: i32, y: i32, z: i32) {
    var a = 1 + x + y + z;
    a = a + 2;
}

fn b() {
    c(1, 2, 3);
    c(4, 5, 6);
}
