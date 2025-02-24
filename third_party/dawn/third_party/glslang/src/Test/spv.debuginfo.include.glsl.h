
out vec4 headerOut;

uniform UBO {
	vec4 headerUboItem;
};

vec4 headerFunction(vec4 a) {
	return -a;
}