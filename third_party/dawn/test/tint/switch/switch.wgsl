fn a() {
    var a = 0;
    switch(a) {
        case 0: {
            break;
        }
        case 1: {
            return;
        }
        default: {
            a += 2;
            break;
        }
    }
}
