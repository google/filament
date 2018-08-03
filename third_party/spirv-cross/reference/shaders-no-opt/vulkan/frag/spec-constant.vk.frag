#version 310 es
precision mediump float;
precision highp int;

struct Foo
{
    float elems[(4 + 2)];
};

layout(location = 0) out vec4 FragColor;

void main()
{
    float t0 = 1.0;
    float t1 = 2.0;
    mediump uint c0 = (uint(3) + 0u);
    mediump int c1 = (-3);
    mediump int c2 = (~3);
    mediump int c3 = (3 + 4);
    mediump int c4 = (3 - 4);
    mediump int c5 = (3 * 4);
    mediump int c6 = (3 / 4);
    mediump uint c7 = (5u / 6u);
    mediump int c8 = (3 % 4);
    mediump uint c9 = (5u % 6u);
    mediump int c10 = (3 >> 4);
    mediump uint c11 = (5u >> 6u);
    mediump int c12 = (3 << 4);
    mediump int c13 = (3 | 4);
    mediump int c14 = (3 ^ 4);
    mediump int c15 = (3 & 4);
    bool c16 = (false || true);
    bool c17 = (false && true);
    bool c18 = (!false);
    bool c19 = (false == true);
    bool c20 = (false != true);
    bool c21 = (3 == 4);
    bool c22 = (3 != 4);
    bool c23 = (3 < 4);
    bool c24 = (5u < 6u);
    bool c25 = (3 > 4);
    bool c26 = (5u > 6u);
    bool c27 = (3 <= 4);
    bool c28 = (5u <= 6u);
    bool c29 = (3 >= 4);
    bool c30 = (5u >= 6u);
    mediump int c31 = c8 + c3;
    mediump int c32 = int(5u + 0u);
    bool c33 = (3 != int(0u));
    bool c34 = (5u != 0u);
    mediump int c35 = int(false);
    mediump uint c36 = uint(false);
    float c37 = float(false);
    float vec0[(3 + 3)][8];
    float vec1[(3 + 2)];
    Foo foo;
    FragColor = ((vec4(t0 + t1) + vec4(vec0[0][0])) + vec4(vec1[0])) + vec4(foo.elems[3]);
}

