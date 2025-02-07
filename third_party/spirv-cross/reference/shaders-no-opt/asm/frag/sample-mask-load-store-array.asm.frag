#version 450

void main()
{
    int _21_unrolled[1];
    for (int i = 0; i < int(1); i++)
    {
        _21_unrolled[i] = gl_SampleMaskIn[i];
    }
    int copy_sample_mask[1] = _21_unrolled;
    for (int i = 0; i < int(1); i++)
    {
        gl_SampleMask[i] = copy_sample_mask[i];
    }
}

