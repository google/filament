#version 450
#extension GL_EXT_tessellation_shader : require

layout(triangles) in;
layout(location = 0) in struct {
  float dummy;
  vec4 variableInStruct;
} testStructArray[][3];
layout(location = 0) out float outResult;
void main(void)
{
  gl_Position = vec4(gl_TessCoord.xy * 2.0 - 1.0, 0.0, 1.0);
  float result;
  result = float(abs(testStructArray[0][2].variableInStruct.x - -4.0) < 0.001) *
		   float(abs(testStructArray[0][2].variableInStruct.y - -9.0) < 0.001) *
		   float(abs(testStructArray[0][2].variableInStruct.z - 3.0) < 0.001) *
		   float(abs(testStructArray[0][2].variableInStruct.w - 7.0) < 0.001);
  outResult = result;
}
