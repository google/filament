fn f() -> i32 {
    var i : i32;
    var j : i32;
    loop {
        if (i > 4) {
            return 1;
        }
        loop {
            if (j > 4) {
                return 2;
            }
            continuing {
                j = j + 1;
            }
        }
        continuing {
            i = i + 1;
        }
    }
}
