#version 430 core
#extension GL_ARB_shader_storage_buffer_object :     enable

uniform vec4 		objectColour;
layout(r8ui) uniform uimage2D 	mainTex;
uniform sampler2DShadow shadowTex;

uniform vec3	lightPos;
uniform float	lightRadius;
uniform vec4	lightColour;

uniform vec3	cameraPos;

uniform bool hasTexture;

uniform bool isPaintable;

uniform int width;
uniform int height;

layout(std430, binding = 4) buffer PaintSSBO{
	int paintData[];
};

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColor;

bool isAdjacentToInk(){
	float ut = (IN.texCoord.x * width) - floor(IN.texCoord.x * width);
	float vt = (IN.texCoord.y * height) - floor(IN.texCoord.y * height);
	int u = int(floor(IN.texCoord.x * width));
	int v = int(floor(IN.texCoord.y * height));
	int dataIndex;
	if(ut < 0.5f && vt < 0.5f){
		u = max(u-1,0);
		v = max(v-1,0);
		dataIndex = v * width + u;
	}
	if(ut > 0.5f && vt < 0.5f){
		u = min(u+1,width-1);
		v = max(v-1,0);
		dataIndex = v * width + u;
	}
	if(ut < 0.5f && vt > 0.5f){
		u = max(u-1,0);
		v = min(v+1,height-1);
		dataIndex = v * width + u;
	}
	if(ut > 0.5f && vt > 0.5f){
		u = min(u+1,width-1);
		v = min(v+1,height-1);
		dataIndex = v * width + u;
	}
	
	return paintData[dataIndex] == 1;
}


// 2D Random
float random(in vec2 st) {
    return fract(sin(dot(st.xy,
        vec2(12.9898, 78.233)))
        * 43758.5453123);
}

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise(in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f * f * (3.0 - 2.0 * f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
        (c - a) * u.y * (1.0 - u.x) +
        (d - b) * u.x * u.y;
}


float fbm(in vec2 st) {
	const int OCTAVES = 5;

	float amplitude = 0.5;
	float frequency = 1.0;
	float value = 0.0;

	for (int i = 0; i < OCTAVES; i++) {
		value += amplitude * noise(frequency * st);

		frequency *= 3.0;
		amplitude *= 0.75;
	}

	return value;
}

vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec2 fade(vec2 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}

float cnoise(vec2 P){
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod(Pi, 289.0); // To avoid truncation effects in permutation
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;
  vec4 i = permute(permute(ix) + iy);
  vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0; // 1/41 = 0.024...
  vec4 gy = abs(gx) - 0.5;
  vec4 tx = floor(gx + 0.5);
  gx = gx - tx;
  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);
  vec4 norm = 1.79284291400159 - 0.85373472095314 * 
    vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
  g00 *= norm.x;
  g01 *= norm.y;
  g10 *= norm.z;
  g11 *= norm.w;
  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));
  vec2 fade_xy = fade(Pf.xy);
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
  return 2.3 * n_xy;
}

vec3 hsv2rgb(vec3 c) {
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 teamColor(uint v) {
	if (v == 0) return vec3(0, 0, 0);

	float c = fract(float(v) * 0.1415);
	return hsv2rgb(vec3(c, 0.8, 1.0));
}

vec2 starNoise(in vec2 st) {
	vec2 uv= 2.0 * st - 1.0;
    
    float r = length(uv);
    float a = atan(uv.y, uv.x);
    
    return vec2(step(r + 0.1 * sin(8.0 * a), 0.7)) * vec2(r, r);
}

void main(void)
{
	float shadow = 1.0; // New !
	
	if( IN . shadowProj . w > 0.0) { // New !
		shadow = textureProj ( shadowTex , IN . shadowProj ) * 0.5f;
	}

	vec3  incident = normalize ( lightPos - IN.worldPos );
	float lambert  = max (0.0 , dot ( incident , IN.normal )) * 0.9; 
	
	vec3 viewDir = normalize ( cameraPos - IN . worldPos );
	vec3 halfDir = normalize ( incident + viewDir );

	float rFactor = max (0.0 , dot ( halfDir , IN.normal ));
	float sFactor = pow ( rFactor , 80.0 );
	
	vec4 albedo = IN.colour;
	
	/*if(hasTexture) {
	 albedo *= texture(mainTex, IN.texCoord);
	}*/
	
	albedo.rgb = pow(albedo.rgb, vec3(2.2));
	
	fragColor.rgb = albedo.rgb * 0.05f; //ambient
	
	fragColor.rgb += albedo.rgb * lightColour.rgb * lambert * shadow; //diffuse light
	
	fragColor.rgb += lightColour.rgb * sFactor * shadow; //specular light
	
	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2f));
	
	fragColor.a = albedo.a;

	fragColor.rbg += height + width;
	fragColor = vec4(0);
	fragColor.a = 1;
	//fragColor.rg = IN.texCoord;

	

	int dataIndex = int(floor(IN.texCoord.y * height)) * width + int(floor(IN.texCoord.x * width));
	/*if(paintData[dataIndex] == 1){fragColor = vec4(0,0,1,1);}
	else{
		if(isAdjacentToInk()){fragColor = vec4(0.5,0.5,1,1);}
		else{
			//fragColor = vec4((IN.texCoord.x * width) - floor(IN.texCoord.x * width),(IN.texCoord.y * height) - floor(IN.texCoord.y * height),0,1);
			fragColor = vec4((IN.texCoord.x),(IN.texCoord.y),0,1);
		}
	}*/
	
	fragColor = vec4(0,0,0,1);

	//
	const float noiseScale = 10.0;

	/*vec2 offset = vec2(fbm(IN.texCoord * noiseScale), 0);
	offset.y = fbm((IN.texCoord + 0.5 * vec2(offset.x))  * noiseScale);
	offset = 2.0 * offset + 1.0;
	offset *= 0.1;

	//fragColor.rgb = vec3(offset, 0);

	ivec2 sampleCoords = ivec2(imageSize(mainTex) * (IN.texCoord + offset));
	if(imageLoad(mainTex,sampleCoords).r != 0){fragColor.rgb = vec3(1);}
	fragColor.a = 1;*/

#if 0
	uint centerValue = imageLoad(mainTex,ivec2(imageSize(mainTex) * IN.texCoord) + ivec2(0, 0)).r;
	uint leftValue =   imageLoad(mainTex,ivec2(imageSize(mainTex) * IN.texCoord) + ivec2(-1, 0)).r;
	uint rightValue =  imageLoad(mainTex,ivec2(imageSize(mainTex) * IN.texCoord) + ivec2(1, 0)).r;
	uint topValue =    imageLoad(mainTex,ivec2(imageSize(mainTex) * IN.texCoord) + ivec2(0, 1)).r;
	uint bottomValue = imageLoad(mainTex,ivec2(imageSize(mainTex) * IN.texCoord) + ivec2(0, -1)).r;

	float coverage = 1.0;
	if (centerValue == 0) {
		coverage = 0.0;

		if (leftValue != 0 && centerValue != leftValue) coverage += 0.25;
		if (rightValue != 0 && centerValue != rightValue) coverage += 0.25;
		if (topValue != 0 && centerValue != topValue) coverage += 0.25;
		if (bottomValue != 0 && centerValue != bottomValue) coverage += 0.25;
	}

	fragColor.rgb = vec3(coverage);
	fragColor.a = 1;
#elif 1
	vec2 offset = vec2(fbm(IN.texCoord * noiseScale), 0);
		offset.y = fbm((1.0 - IN.texCoord)  * noiseScale);
	offset = 2.0 * offset + 1.0;
	offset *= 0.04;

	vec2 realCoords = imageSize(mainTex) * (IN.texCoord + offset);
	vec2 iCoords = floor(realCoords);
	vec2 fCoords = fract(realCoords);
	
	uint value00 = imageLoad(mainTex, ivec2(iCoords) + ivec2(0, 0)).r;
	uint value10 = imageLoad(mainTex, ivec2(iCoords) + ivec2(1, 0)).r;
	uint value01 = imageLoad(mainTex, ivec2(iCoords) + ivec2(0, 1)).r;
	uint value11 = imageLoad(mainTex, ivec2(iCoords) + ivec2(1, 1)).r;

	vec3 a = teamColor(value00);
    vec3 b = teamColor(value10);
    vec3 c = teamColor(value01);
    vec3 d = teamColor(value11);

    vec2 u = fCoords;// * fCoords * (3.0 - 2.0 * fCoords);

    vec3 color = mix(a, b, u.x) +
        (c - a) * u.y * (1.0 - u.x) +
        (d - b) * u.x * u.y;
		
	fragColor.rgb = color;
	fragColor.a = 1;

	
#else

#endif

//fragColor.rgb = IN.normal;

	//fragColor = IN.colour;
	
	//fragColor.xy = IN.texCoord.xy;
	
	//fragColor = IN.colour;
}

