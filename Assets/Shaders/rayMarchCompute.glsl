#version 450 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
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
uniform float debugValue;
uniform bool depthTest;


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
	vec3 position;
	vec3 color;
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


//credit - sebastian lague
vec4 Blend( float a, float b, vec3 colA, vec3 colB, float k )
{
    float h = clamp( 0.5+0.5*(b-a)/k, 0.0, 1.0 );
    float blendDst = mix( b, a, h ) - k*h*(1.0-h);
    vec3 blendCol = mix(colB,colA,h);
    return vec4(blendCol, blendDst);
}


float SDFJustDistance(vec3 from){
	return 0;
}


HitInformation SDF(vec3 from) {
	
	HitInformation hit;
	hit.closestDistance = 99999.;
	hit.color = vec3(0);
	for (int i = 0; i < numSpheres; i++)
	{
		Sphere sphere = sphereData[i];
		vec3 sphereCenter = vec3(sphere.x,sphere.y,sphere.z);
		float sphereRadius = sphere.radius;
		vec3 nextCol = vec3(sphere.r,sphere.g,sphere.b);
		float newDistance = length(from - sphereCenter) - (sphereRadius);
		vec4 result = Blend(hit.closestDistance,newDistance,hit.color,nextCol,10);
		hit.closestDistance = result.w;
		hit.color =  result.rgb;
		hit.position = from;
		//if (newDistance < hit.closestDistance) {
			//hit.closestDistance = newDistance;
			//hit.sphereID = i;
			//hit.position = from;
		//}
	}
	return hit;
}

//https://www.shadertoy.com/view/XlGBW3
vec3 GetNormal(vec3 p) {
	float d = SDF(p).closestDistance;
    vec2 e = vec2(.0005, 0);
    
    vec3 n = d - vec3(
        SDF(p-e.xyy).closestDistance,
        SDF(p-e.yxy).closestDistance,
        SDF(p-e.yyx).closestDistance);
    
    return normalize(n);
}


vec4 RayMarch(vec3 rayDir) {
	vec3 lightPos = vec3(0, 100, 20);//stop hardcoding
	float distanceFromOrigin = 0;
	for (int i = 0; i < maxSteps; i++)
	{
		vec3 nextPointOnLine = cameraPos + distanceFromOrigin * rayDir;
		HitInformation hit;
		hit = SDF(nextPointOnLine);
		distanceFromOrigin += hit.closestDistance;
		if (hit.closestDistance < hitDistance) {
			
			
			//vec3 normal = normalize(hit.position - sphereCenter);
			vec3 normal = GetNormal(hit.position);
			//normal = (normal+1)/2;
			vec3 lightDir = normalize(lightPos - hit.position);
			hit.color = hit.color * max(dot(lightDir, normal), 0.05);//want there to be at least a little bit of colour
			
			return vec4(hit.color, distanceFromOrigin);
		}
		if (distanceFromOrigin > farPlane) {//ray has hit nothing
			return vec4(0,0,0,-1);
		}
	}
	return vec4(1, 0, 0, -1);//todo handle max steps reached
}


void main() {
	ivec2 IMAGE_COORD = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
	if(IMAGE_COORD.x >= viewportWidth || IMAGE_COORD.y >= viewportHeight){return;}
	vec2 texCoord = vec2(gl_GlobalInvocationID.x/float(viewportWidth), gl_GlobalInvocationID.y/float(viewportHeight));
	texCoord = vec2(texCoord.y, -texCoord.x);//thank you jason
	vec3 rayDir = RayDir(vec2(IMAGE_COORD.x/float(viewportWidth),IMAGE_COORD.y/float(viewportHeight)));
	
	vec4 result = RayMarch(rayDir);//rgb = colour, a = distance from camera

	float sceneDistanceFromCamera;
	//get existing scene distance from depth texture and projMatrix
	if(depthTest){
		float depth = texture(depthTex, texCoord).r;
		vec4 clipSpace = vec4(texCoord.x, texCoord.y, depth, 1) * 2 - 1;
		vec4 viewSpace = inverse(projMatrix) * clipSpace;
		sceneDistanceFromCamera = -(viewSpace.z / viewSpace.w);
	}
	else{
		sceneDistanceFromCamera = 1./0.;
	}
	

	if (result.a < sceneDistanceFromCamera && result.a != -1) {
		//sphere was hit and depth test passed
		result.w = 0.9;//will need to rework if we want transparent spheres
		imageStore(tex, IMAGE_COORD, result);
	}
	else {
		//sphere wasn't hit or depth test failed, draw blank pixel
		imageStore(tex, IMAGE_COORD, vec4(0));
	}
	return;


}