#version 400 core
#extension GL_ARB_shader_storage_buffer_object :     enable

uniform sampler2D sceneTex;
uniform sampler2D hdrTex;

uniform float bloomStrength = 0.04f;

in Vertex {
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColor;

vec3 computeBloomMix() {
    vec3 hdr = texture(sceneTex, IN.texCoord).rgb;
    vec3 blm = texture(hdrTex, IN.texCoord).rgb;
    vec3 col = mix(hdr, blm, vec3(bloomStrength));
    return col;
}

void main(void) {
	fragColor = vec4(computeBloomMix(), 1.0);
}
