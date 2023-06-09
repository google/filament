#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

fragment void main0()
{
    long sw = 42l;
    int result = 0;
    switch (sw)
    {
        case -42l:
        {
            result = 42;
        }
        case 420l:
        {
            result = 420;
        }
        case -34359738368l:
        {
            result = 420;
            break;
        }
    }
}

