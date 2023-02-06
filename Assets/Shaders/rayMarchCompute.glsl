#version 430 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(r8, binding = 0) uniform image2D tex;
uniform sampler2D depthTex;


uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 cameraPos;
uniform int maxSteps;
uniform float hitDistance;
uniform float noHitDistance;
uniform int viewportWidth;
uniform int viewportHeight;


uniform float nearPlane;
uniform float farPlane;

uniform int numSpheres;

struct Sphere {
	float x, y, z;
	float radius;
	float r, g, b;
};
layout(std430, binding = 4) buffer SphereSSBO {
	Sphere sphereData[];
};

struct HitInformation {
	float closestDistance;
	int sphereID;
	vec3 position;
};

vec3 RayDir(vec2 pixel)//takes pixel in 0,1 range
{
	vec2 ndc = pixel * 2 - 1;//move from 0,1 to -1,1
	ndc = vec2(ndc.y, -ndc.x);//thank you jason

	vec4 clipSpace = vec4(ndc, -1,1);

	vec4 viewSpace = inverse(projMatrix) * clipSpace;
	viewSpace.w = 0;

	vec3 worldSpace = (inverse(viewMatrix) * viewSpace).xyz;

	return normalize(worldSpace);
}

HitInformation SDF(vec3 from) {
	
	HitInformation hit;
	hit.closestDistance = 1. / 0.;
	for (int i = 0; i < numSpheres; i++)
	{
		Sphere sphere = sphereData[i];
		vec3 sphereCenter = vec3(sphere.x,sphere.y,sphere.z);
		float sphereRadius = sphere.radius;
		float newDistance = length(from - sphereCenter) - (sphereRadius);
		if (newDistance < hit.closestDistance) {
			hit.closestDistance = newDistance;
			hit.sphereID = i;
			hit.position = from;
		}
	}
	return hit;
}

vec4 RayMarch(vec3 rayDir) {
	vec3 lightPos = vec3(0, 40, 0);//stop hardcoding
	float distanceFromOrigin = 0;
	for (int i = 0; i < maxSteps; i++)
	{
		vec3 nextPointOnLine = cameraPos + distanceFromOrigin * rayDir;
		HitInformation hit;
		hit = SDF(nextPointOnLine);
		distanceFromOrigin += hit.closestDistance;
		if (hit.closestDistance < hitDistance) {
			Sphere sphere = sphereData[hit.sphereID];
			vec3 sphereCenter = vec3(sphere.x, sphere.y, sphere.z);
			vec3 color = vec3(sphere.r, sphere.g, sphere.b);
			vec3 normal = normalize(hit.position - sphereCenter);
			vec3 lightDir = normalize(lightPos - hit.position);
			color = color * max(dot(lightDir, normal), 0.05);//want there to be at least a little bit of colour
			
			return vec4(color, distanceFromOrigin);
		}
		if (distanceFromOrigin > noHitDistance) {
			return vec4(0,0,0,-1);
		}
	}
	return vec4(1, 0, 0, -1);//todo handle max steps reached
}


void main() {
	ivec2 IMAGE_COORD = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
	vec2 texCoord = vec2(gl_GlobalInvocationID.x/float(viewportWidth), gl_GlobalInvocationID.y/float(viewportHeight));
	texCoord = vec2(texCoord.y, -texCoord.x);//thank you jason
	vec3 rayDir = RayDir(vec2(IMAGE_COORD.x/float(viewportWidth),IMAGE_COORD.y/float(viewportHeight)));
	
	vec4 result = RayMarch(rayDir);//rgb = colour, a = distance from camera

	//get existing scene distance from depth texture and projMatrix
	float depth = texture(depthTex, texCoord).r;
	vec4 clipSpace = vec4(texCoord.x, texCoord.y, depth, 1) * 2 - 1;
	vec4 viewSpace = inverse(projMatrix) * clipSpace;
	float sceneDistanceFromCamera = -(viewSpace.z / viewSpace.w);

	if (result.a < sceneDistanceFromCamera && result.a != -1) {
		//sphere was hit and depth test passed
		result.w = 1;//will need to rework if we want transparent spheres
		imageStore(tex, IMAGE_COORD, result);
	}
	else {
		//sphere wasn't hit or depth test failed, draw blank pixel
		imageStore(tex, IMAGE_COORD, vec4(0));
	}
	return;


}