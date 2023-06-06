#version 450

void main()
{
    float constituent = float(0);
    mat3 _mat3 = mat3(vec3(constituent), vec3(constituent), vec3(constituent));
    float constituent_1 = float(1);
    _mat3 = mat3(vec3(constituent_1), vec3(constituent_1), vec3(constituent_1));
}

