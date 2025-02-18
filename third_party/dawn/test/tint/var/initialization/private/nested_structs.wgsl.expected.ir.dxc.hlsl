struct S1 {
  int i;
};

struct S2 {
  S1 s1;
};

struct S3 {
  S2 s2;
};


static const S1 v = {int(42)};
static const S2 v_1 = {v};
static const S3 v_2 = {v_1};
static S3 P = v_2;
RWByteAddressBuffer v_3 : register(u0);
[numthreads(1, 1, 1)]
void main() {
  v_3.Store(0u, asuint(P.s2.s1.i));
}

