#version 400 core
#define mad(a, b, c) (a * b + c)

layout(location = 0) in vec3 position;

uniform int width;
uniform int height;

out Vertex
{
	vec2 texCoord;
	vec4 offset[3];
} OUT;

void main() {
	vec2 resolution = vec2(width,height);
  vec4 SMAA_RT_METRICS = vec4(1.0 / resolution.x, 1.0 / resolution.y, resolution.x, resolution.y);

	OUT.texCoord = vec2((position + 1.0) / 2.0);
	//OUT.texCoord.y = 1 - OUT.texCoord.y;

  OUT.offset[0] = mad(SMAA_RT_METRICS.xyxy, vec4(-1.0, 0.0, 0.0, -1.0), OUT.texCoord.xyxy);
  OUT.offset[1] = mad(SMAA_RT_METRICS.xyxy, vec4( 1.0, 0.0, 0.0,  1.0), OUT.texCoord.xyxy);
  OUT.offset[2] = mad(SMAA_RT_METRICS.xyxy, vec4(-2.0, 0.0, 0.0, -2.0), OUT.texCoord.xyxy);

  gl_Position = vec4(position, 1.0);
}