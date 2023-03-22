#version 400 core

in vec4 outColor;
out vec4 fragColor;

void main(void) {
	fragColor = outColor;
}