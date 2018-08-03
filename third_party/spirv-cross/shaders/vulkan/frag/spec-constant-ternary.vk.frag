#version 450
layout(location = 0) out float FragColor;
layout(constant_id = 0) const uint s = 10u;
const uint f = s > 20u ? 30u : 50u;

void main()
{
	FragColor = float(f);
}
