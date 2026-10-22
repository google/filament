#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

fragment void main0()
{
    int sw0 = 42;
    int result = 0;
    switch (sw0)
    {
        case -42:
        {
            result = 42;
        }
        case 420:
        {
            result = 420;
        }
        case -1234:
        {
            result = 420;
            break;
        }
    }
    char sw1 = char(10);
    switch (sw1)
    {
        case -42:
        {
            result = 42;
        }
        case 42:
        {
            result = 420;
        }
        case -123:
        {
            result = 512;
            break;
        }
    }
    short sw2 = short(10);
    switch (sw2)
    {
        case -42:
        {
            result = 42;
        }
        case 42:
        {
            result = 420;
        }
        case -1234:
        {
            result = 512;
            break;
        }
    }
    short sw3 = short(10);
    switch (sw3)
    {
        case -42:
        {
            result = 42;
        }
        case 42:
        {
            result = 420;
        }
        case -1234:
        {
            result = 512;
            break;
        }
    }
}

