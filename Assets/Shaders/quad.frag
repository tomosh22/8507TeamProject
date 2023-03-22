#version 400 core
#extension GL_ARB_shader_storage_buffer_object :     enable

uniform sampler2D mainTex;
uniform bool hasTexture;

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColor;

void main(void) {
	if(hasTexture) {
		fragColor = texture(mainTex,IN.texCoord);
	} else { fragColor = IN.colour; }
}