#version 400 core

#define mad(a, b, c) (a * b + c)




#define SMAA_MAX_SEARCH_STEPS 112



layout(location = 0) in vec3 position;

uniform int width;
uniform int height;



out Vertex
{
	vec2 texCoord;
	vec2 pixelCoord;
	vec4 offset[3];
} OUT;

void main() {
	vec2 resolution = vec2(width,height);
  vec4 SMAA_RT_METRICS = vec4(1.0 / resolution.x, 1.0 / resolution.y, resolution.x, resolution.y);

	OUT.texCoord = vec2((position.xy + 1.0) / 2.0);
	//OUT.texCoord.y = 1 - OUT.texCoord.y;

  OUT.pixelCoord = OUT.texCoord * SMAA_RT_METRICS.zw;

  // We will use these offsets for the searches later on (see @PSEUDO_GATHER4):
  OUT.offset[0] = mad(SMAA_RT_METRICS.xyxy, vec4(-0.25, -0.125,  1.25, -0.125), OUT.texCoord.xyxy);
  OUT.offset[1] = mad(SMAA_RT_METRICS.xyxy, vec4(-0.125, -0.25, -0.125,  1.25), OUT.texCoord.xyxy);

  // And these for the searches, they indicate the ends of the loops:
  OUT.offset[2] = mad(
    SMAA_RT_METRICS.xxyy,
    vec4(-2.0, 2.0, -2.0, 2.0) * float(SMAA_MAX_SEARCH_STEPS),
    vec4(OUT.offset[0].xz, OUT.offset[1].yw)
  );

  gl_Position = vec4(position, 1.0);
}