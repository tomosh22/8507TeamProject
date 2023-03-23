#version 420 core
layout(vertices =3) out; //num vertices in output patch

in Vertex {
	vec4 colour;
	vec2 texCoord;
	vec2 texCoordPBR;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
	vec3 tangent;
	vec3 binormal; //From Vertex Shader
} IN[]; //Equal to size of the draw call vertex count

out Vertex {
	vec4 colour;
	vec2 texCoord;
	vec2 texCoordPBR;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
	vec3 tangent;
	vec3 binormal; //To Evaluation Shader
} OUT []; // Equal to the size of the layout vertex count
void main() {
	gl_TessLevelInner [0] = 8;
	gl_TessLevelInner [1] = 8;
	gl_TessLevelOuter [0] = 8;
	gl_TessLevelOuter [1] = 8;
	gl_TessLevelOuter [2] = 8;
	gl_TessLevelOuter [3] = 8;

	OUT[gl_InvocationID].texCoord = IN[gl_InvocationID].texCoord;
	OUT[gl_InvocationID].texCoordPBR = IN[gl_InvocationID].texCoordPBR;
	OUT[gl_InvocationID].shadowProj = IN[gl_InvocationID].shadowProj;
	OUT[gl_InvocationID].normal = IN[gl_InvocationID].normal;
	OUT[gl_InvocationID].worldPos = IN[gl_InvocationID].worldPos;
	OUT[gl_InvocationID].tangent = IN[gl_InvocationID].tangent;
	OUT[gl_InvocationID].binormal = IN[gl_InvocationID].binormal;

	gl_out[gl_InvocationID ]. gl_Position = gl_in[gl_InvocationID]. gl_Position;
 }