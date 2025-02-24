@compute @workgroup_size(1)
fn f() {
    var i : i32 = 0;
    loop {
        switch (i) {
            case 0: {
                continue;
            }
            default:{
                break;
            }
        }
        continuing {
           i = i + 1;
           break if i >= 4;
        }
    }
}
