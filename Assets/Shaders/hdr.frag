#version 400 core

uniform sampler2D 	mainTex;



uniform int width;
uniform int height;


in Vertex
{
	vec2 texCoord;
} IN;

out vec4 fragColor;


void main(void)
{
	fragColor = texture(mainTex,IN.texCoord);
}


