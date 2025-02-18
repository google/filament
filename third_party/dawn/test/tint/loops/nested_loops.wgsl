fn f() -> i32 {
    var i : i32;
    var j : i32;
    loop {
        i = i + 1;
        if (i > 4) {
            return 1;
        }
        loop {
            j = j + 1;
            if (j > 4) {
                return 2;
            }
        }
    }
}
