#version 330
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif

void main()
{
    int sw = 42;
    int result = 0;
    switch (sw)
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
}

