#version 400

struct B {
    int a;
};

// Test struct parameter after non-struct parameter (minimal regression test).
void test1(B b, int c) {} // works.
void test2(int c, B b) {} // should work.

void main() {
    B b = B(42);
    test1(b, 10);
    test2(10, b);
} 