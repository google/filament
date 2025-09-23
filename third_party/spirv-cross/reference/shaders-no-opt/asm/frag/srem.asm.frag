#version 450

layout(location = 0) out int oA;
layout(location = 0) flat in int A;
layout(location = 1) flat in uint B;
layout(location = 1) out uint oB;
layout(location = 2) flat in int C;
layout(location = 3) flat in uint D;

void main()
{
    oB = uint(A - int(B) * (A / int(B)));
    oB = uint(A - C * (A / C));
    oB = uint(int(B) - int(D) * (int(B) / int(D)));
    oB = uint(int(B) - A * (int(B) / A));
    oA = A - int(B) * (A / int(B));
    oA = A - C * (A / C);
    oA = int(B) - int(D) * (int(B) / int(D));
    oA = int(B) - A * (int(B) / A);
}

