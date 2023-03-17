#version 400 core

uniform sampler2D tex;

uniform int width;
uniform int height;


in Vertex
{
	vec2 texCoord;
} IN;

out vec3 fragColor;

void main( void ) {
    vec2 resolution = vec2(width,height);
    vec2 srcTexelSize = 1.0 / resolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(tex, vec2(IN.texCoord.x - 2*x, IN.texCoord.y + 2*y)).rgb;
    vec3 b = texture(tex, vec2(IN.texCoord.x,       IN.texCoord.y + 2*y)).rgb;
    vec3 c = texture(tex, vec2(IN.texCoord.x + 2*x, IN.texCoord.y + 2*y)).rgb;

    vec3 d = texture(tex, vec2(IN.texCoord.x - 2*x, IN.texCoord.y)).rgb;
    vec3 e = texture(tex, vec2(IN.texCoord.x,       IN.texCoord.y)).rgb;
    vec3 f = texture(tex, vec2(IN.texCoord.x + 2*x, IN.texCoord.y)).rgb;

    vec3 g = texture(tex, vec2(IN.texCoord.x - 2*x, IN.texCoord.y - 2*y)).rgb;
    vec3 h = texture(tex, vec2(IN.texCoord.x,       IN.texCoord.y - 2*y)).rgb;
    vec3 i = texture(tex, vec2(IN.texCoord.x + 2*x, IN.texCoord.y - 2*y)).rgb;

    vec3 j = texture(tex, vec2(IN.texCoord.x - x, IN.texCoord.y + y)).rgb;
    vec3 k = texture(tex, vec2(IN.texCoord.x + x, IN.texCoord.y + y)).rgb;
    vec3 l = texture(tex, vec2(IN.texCoord.x - x, IN.texCoord.y - y)).rgb;
    vec3 m = texture(tex, vec2(IN.texCoord.x + x, IN.texCoord.y - y)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1
    fragColor = e*0.125;
    fragColor += (a+c+g+i)*0.03125;
    fragColor += (b+d+f+h)*0.0625;
    fragColor += (j+k+l+m)*0.125;
    
}