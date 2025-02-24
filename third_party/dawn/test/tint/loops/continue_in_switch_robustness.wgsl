@compute @workgroup_size(1)
fn f() {
    for (var i : i32 = 0; i < 4; i = i + 1) {
        switch(i) {
            case 0: {
                continue;
            }
            default:{
                break;
            }
        }
    }
}
