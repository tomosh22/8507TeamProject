#version 400 core
precision highp float;

//#ifndef SMAA_THRESHOLD
//#define SMAA_THRESHOLD 0.1
//#endif

uniform float SMAA_THRESHOLD;

#ifndef SMAA_DEPTH_THRESHOLD
#define SMAA_DEPTH_THRESHOLD (0.1 * SMAA_THRESHOLD)
#endif

uniform sampler2D depthTex;

in Vertex
{
	vec2 texCoord;
	vec4 offset[3];
} IN;

/**
 * Gathers current pixel, and the top-left neighbors.
 */
vec3 SMAAGatherNeighbours(vec2 texcoord, vec4 offset[3], sampler2D tex) {
  float P = texture2D(tex, texcoord).r;
  float Pleft = texture2D(tex, offset[0].xy).r;
  float Ptop  = texture2D(tex, offset[0].zw).r;

  return vec3(P, Pleft, Ptop);
}

out vec4 fragColor;
void main () {
  vec3 neighbours = SMAAGatherNeighbours(IN.texCoord, IN.offset, depthTex);
  vec2 delta = abs(neighbours.xx - vec2(neighbours.y, neighbours.z));
  vec2 edges = step(SMAA_DEPTH_THRESHOLD, delta);

  if (dot(edges, vec2(1.0, 1.0)) == 0.0)
      discard;

  fragColor = vec4(edges, 0.0, 1.0);
}