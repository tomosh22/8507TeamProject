#version 400 core
#define mad(a, b, c) (a * b + c)

layout(location = 0) in vec3 position;

uniform int width;
uniform int height;
vec2 resolution = vec2(width,height);

out Vertex
{
	vec2 texCoord;
	vec4 offset;
} OUT;

void main() {
  vec4 SMAA_RT_METRICS = vec4(1.0 / resolution.x, 1.0 / resolution.y, resolution.x, resolution.y);

	OUT.texCoord = vec2((position.xy + 1.0) / 2.0);
	OUT.offset = mad(SMAA_RT_METRICS.xyxy, vec4(1.0, 0.0, 0.0,  1.0), OUT.texCoord.xyxy);

  gl_Position = vec4(position, 1.0);
}