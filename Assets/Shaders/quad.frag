#version 400 core
#extension GL_ARB_shader_storage_buffer_object :     enable

uniform vec4 		objectColour;
uniform sampler2D 	mainTex;
uniform sampler2DShadow shadowTex;

uniform vec3	lightPos;
uniform float	lightRadius;
uniform vec4	lightColour;

uniform vec3	cameraPos;

uniform bool hasTexture;

uniform bool isPaintable;


uniform int width;
uniform int height;


in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColor[2];


void main(void)
{
	if(hasTexture){
		fragColor[0] = texture(mainTex,IN.texCoord);
		
	}
	else{fragColor[0] = IN.colour;}
	fragColor[1] = vec4(0,0,0,0);//todo why do i need to do this???
}


