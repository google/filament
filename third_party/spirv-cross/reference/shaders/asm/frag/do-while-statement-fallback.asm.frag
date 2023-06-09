#version 450

layout(location = 0) out float FragColor;

void main()
{
    float foo = 1.0;
    for (;;)
    {
        foo = 2.0;
        if (false)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    for (;;)
    {
        foo = 3.0;
        if (false)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    for (;;)
    {
        foo = 4.0;
        if (false)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    for (;;)
    {
        foo = 5.0;
        if (false)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    FragColor = foo;
}

