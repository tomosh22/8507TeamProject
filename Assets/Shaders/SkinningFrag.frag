#version 330 core
uniform sampler2D diffuseTex;
in Vertex {
vec2 texCoord;
} IN;

out vec4 fragColour[2];
void main(void) {
	fragColour[0] = texture(diffuseTex,IN.texCoord);
	fragColour[1] = vec4(0);
	//fragColour = vec4(IN.texCoord,0,1);
}