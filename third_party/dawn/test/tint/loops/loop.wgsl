fn f() -> i32 {
    var i : i32;
    loop {
        i = i + 1;
        if (i > 4) {
            return i;
        }
    }
}
