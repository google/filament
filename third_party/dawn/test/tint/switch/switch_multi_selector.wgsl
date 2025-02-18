fn a() {
    var a = 0;
    switch(a) {
        case 0, 2, 4: {
            break;
        }
        case 1, default: {
            return;
        }
    }
}
