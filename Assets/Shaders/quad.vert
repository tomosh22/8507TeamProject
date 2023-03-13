#version 400 core

uniform mat4 modelMatrix 	= mat4(1.0f);
uniform mat4 viewMatrix 	= mat4(1.0f);
uniform mat4 projMatrix 	= mat4(1.0f);
uniform mat4 shadowMatrix 	= mat4(1.0f);

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;

uniform vec4 		objectColour = vec4(1,1,1,1);

uniform bool hasVertexColours = false;

uniform bool rotated;

out Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
} OUT;

void main(void)
{
	OUT.texCoord = texCoord;
	if(rotated){
		OUT.texCoord = texCoord.yx;
		OUT.texCoord.y = 1 - OUT.texCoord.y;
	}
	OUT.colour = colour;
	mat4 mvp 		  = (projMatrix * viewMatrix * modelMatrix);
	gl_Position		= vec4(position, 1.0);
}