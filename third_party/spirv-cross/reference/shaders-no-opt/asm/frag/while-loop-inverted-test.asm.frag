#version 450

void main()
{
    int i = 0;
    int j = 0;
    while (!(i == 20))
    {
        j = ((j + i) + 1) * j;
        i++;
    }
}

