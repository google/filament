#version 460

out int outx;
int counter = 0;

void test_if() {
    if (false) {
        counter += 1;
    }
}

void test_ifelse() {
    if (false) {
        counter += 1;
    }
    else {
        counter += 2;
    }
}

void test_if_compound() {
    if (false) {
        if (false) {
            counter += 1;
        }
    }
}

void test_if_compound2() {
    if (false) {
        if (false) {
            counter += 1;
        }

        counter += 2;
    }
}

void test_switch() {
    switch (0) {
    case 0:
        counter += 1;
        // implict fallthrough
    case 1:
        counter += 2;
        break;
    default:
        counter += 3;
        // implicit break
    }
}

void main() {
    test_if();
    test_ifelse();
    test_if_compound();
    test_if_compound2();
    test_switch();
    outx = counter;
}