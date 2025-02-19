#version 450

void main()
{
    uint _16_unrolled[1];
    for (int i = 0; i < int(1); i++)
    {
        _16_unrolled[i] = int(gl_SampleMaskIn[i]);
    }
    uint copy_sample_mask[1] = _16_unrolled;
    for (int i = 0; i < int(1); i++)
    {
        gl_SampleMask[i] = int(copy_sample_mask[i]);
    }
}

