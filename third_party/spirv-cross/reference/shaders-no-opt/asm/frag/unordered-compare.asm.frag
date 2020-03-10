#version 450

layout(location = 0) in vec4 A;
layout(location = 1) in vec4 B;
layout(location = 0) out vec4 FragColor;

vec4 test_vector()
{
    bvec4 le = not(greaterThanEqual(A, B));
    bvec4 leq = not(greaterThan(A, B));
    bvec4 ge = not(lessThanEqual(A, B));
    bvec4 geq = not(lessThan(A, B));
    bvec4 eq = not(notEqual(A, B));
    bvec4 neq = not(equal(A, B));
    return ((((mix(vec4(0.0), vec4(1.0), le) + mix(vec4(0.0), vec4(1.0), leq)) + mix(vec4(0.0), vec4(1.0), ge)) + mix(vec4(0.0), vec4(1.0), geq)) + mix(vec4(0.0), vec4(1.0), eq)) + mix(vec4(0.0), vec4(1.0), neq);
}

float test_scalar()
{
    bool le = !(A.x >= B.x);
    bool leq = !(A.x > B.x);
    bool ge = !(A.x <= B.x);
    bool geq = !(A.x < B.x);
    bool eq = !(A.x != B.x);
    bool neq = !(A.x == B.x);
    return ((((float(le) + float(leq)) + float(ge)) + float(geq)) + float(eq)) + float(neq);
}

void main()
{
    FragColor = test_vector() + vec4(test_scalar());
}

