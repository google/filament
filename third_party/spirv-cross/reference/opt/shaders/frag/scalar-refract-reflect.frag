#version 450

layout(location = 0) out float FragColor;
layout(location = 0) in vec3 vRefract;

void main()
{
    FragColor = refract(vRefract.x, vRefract.y, vRefract.z);
    FragColor += reflect(vRefract.x, vRefract.y);
    FragColor += refract(vRefract.xy, vRefract.yz, vRefract.z).y;
    FragColor += reflect(vRefract.xy, vRefract.zy).y;
}

