#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

fragment void main0()
{
    ulong sw = 42ul;
    int result = 0;
    switch (sw)
    {
        case 42ul:
        {
            result = 42;
        }
        case 420ul:
        {
            result = 420;
        }
        case 343597383680ul:
        {
            result = 420;
            break;
        }
    }
}

