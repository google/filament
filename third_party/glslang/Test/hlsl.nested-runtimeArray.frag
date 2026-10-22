struct A {
    float a[];
};

RWStructuredBuffer <A> B;

float main() {
    return B[0].a[0];
}
