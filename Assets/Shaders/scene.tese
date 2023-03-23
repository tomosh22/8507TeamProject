#version 420

layout(triangles , ccw) in;

layout (binding = 6) uniform sampler2D heightMap;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 scale;

uniform bool useHeightMap;
uniform bool useHeightMapLocal;

uniform float heightMapStrength;

uniform float normalPow;
uniform float worldPosMul;

in Vertex { //Sent from the TCS
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
	vec3 tangent;
	vec3 binormal;
} IN[]; // Equal to TCS layout size

out Vertex { //Each TES works on a single vertex!
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
	vec3 tangent;
	vec3 binormal;
} OUT;

vec3 QuadMixVec3(vec3 a, vec3 b, vec3 c, vec3 d) {
	vec3 p0 = mix(a,c, gl_TessCoord.x);
	vec3 p1 = mix(b,d, gl_TessCoord.x);

	return mix(p0 ,p1 ,gl_TessCoord.y);
}

vec2 QuadMixVec2(vec2 a, vec2 b, vec2 c, vec2 d) {
	vec2 p0 = mix(a,c, gl_TessCoord.x);
	vec2 p1 = mix(b,d, gl_TessCoord.x);

	return mix(p0 ,p1 ,gl_TessCoord.y);
}

vec4 TriMixVec4(vec4 a, vec4 b, vec4 c) {
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	vec4 val0 = a;
	vec4 val1 = b;
	vec4 val2 = c;

	vec4 val = u * val0 + v * val1 + w * val2;
	return val;
}

vec3 TriMixVec3(vec3 a, vec3 b, vec3 c) {
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	vec3 val0 = a;
	vec3 val1 = b;
	vec3 val2 = c;

	vec3 val = u * val0 + v * val1 + w * val2;
	return val;
}

vec2 TriMixVec2(vec2 a, vec2 b, vec2 c) {
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	vec2 val0 = a;
	vec2 val1 = b;
	vec2 val2 = c;

	vec2 val = u * val0 + v * val1 + w * val2;
	return val;
}

vec3 triplanarSample(sampler2D sampler, vec3 worldPos, vec3 normal){
	vec3 xy = texture(sampler, worldPos.xy * worldPosMul).rgb;//todo uniform
	vec3 xz = texture(sampler, worldPos.xz * worldPosMul).rgb;
	vec3 yz = texture(sampler, worldPos.yz * worldPosMul).rgb;

	vec3 myNormal = abs(normal);
	myNormal = pow(myNormal, vec3(normalPow));//todo uniform
	myNormal /= myNormal.x + myNormal.y + myNormal.z;
	vec3 result = xz*myNormal.y + xy*myNormal.z + yz*myNormal.x;
	return result;
}

void main() {
	vec3 combinedPos = TriMixVec3(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, gl_in[2].gl_Position.xyz);
	
	OUT.texCoord = TriMixVec2(IN[0].texCoord, IN[1].texCoord, IN[2].texCoord);
	OUT.shadowProj = TriMixVec4(IN[0].shadowProj, IN[1].shadowProj, IN[2].shadowProj);
	OUT.normal = TriMixVec3(IN[0].normal, IN[1].normal, IN[2].normal);
	OUT.worldPos = TriMixVec3(IN[0].worldPos, IN[1].worldPos, IN[2].worldPos);
	OUT.tangent = TriMixVec3(IN[0].tangent, IN[1].tangent, IN[2].tangent);
	OUT.binormal = TriMixVec3(IN[0].binormal, IN[1].binormal, IN[2].binormal);
	
	vec4 worldPos = modelMatrix * vec4(combinedPos , 1);
	
	if(useHeightMap && useHeightMapLocal){
		float height = triplanarSample(heightMap,OUT.worldPos,OUT.normal).x;
		worldPos.xyz += TriMixVec3(IN[0].normal, IN[1].normal, IN[2].normal) * height * heightMapStrength;
	}
	
	gl_Position = projMatrix * viewMatrix * worldPos;
}