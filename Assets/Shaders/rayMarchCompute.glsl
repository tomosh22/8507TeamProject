#version 430 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(r8, binding = 0) uniform image2D tex;


uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 cameraPos;
uniform int maxSteps;
uniform int hitDistance;
uniform int noHitDistance;
uniform int viewportWidth;
uniform int viewportHeight;



vec3 RayDir(vec2 px)//takes pixel in 0,1 range
{
	vec2 ndc = px * 2. - 1.;//move from 0,1 to -1,1

	vec4 clipSpace = vec4(ndc, -1.,1);

	vec4 viewSpace = inverse(projMatrix) * clipSpace;
	viewSpace.w = 0.;

	vec3 worldSpace = (inverse(viewMatrix) * viewSpace).xyz;

	return normalize(worldSpace);
}

float SDF(vec3 from) {
	vec3 sphereCenter = vec3(0, 0, 0);
	int sphereRadius = 100;
	return length(sphereCenter) - sphereRadius;
}

float RayMarch(vec3 rayDir) {
	float distanceFromOrigin = 0;
	for (int i = 0; i < maxSteps; i++)
	{
		vec3 nextPointOnLine = cameraPos + distanceFromOrigin * rayDir;
		float closestDistance = SDF(nextPointOnLine);
		distanceFromOrigin += closestDistance;
		if (closestDistance < hitDistance || distanceFromOrigin > noHitDistance) {
			break;
		}
	}
	return distanceFromOrigin;
}


void main() {
	ivec2 IMAGE_COORD = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
	if (maxSteps > 500) {
		imageStore(tex, IMAGE_COORD, vec4(0,1,0,0.8));
	}
	else {
		imageStore(tex, IMAGE_COORD, vec4(0, 0, 1, 0.8));
	}
	return;

	vec3 rayDir = RayDir(vec2(IMAGE_COORD.x/float(viewportWidth),IMAGE_COORD.y/float(viewportHeight)));
	//rayDir = (rayDir + 1) / 2; //bring from -1,1 to 0,1
	float result = RayMarch(rayDir)/10;
	imageStore(tex, IMAGE_COORD, vec4(vec3(result), 0.8));
	return;


	

}