#version 450

void main()
{
    int j = 0;
    int i = 0;
    do
    {
        j = ((j + i) + 1) * j;
        i++;
    } while (!(i == 20));
}

