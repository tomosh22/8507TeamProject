#version 430 core
#extension GL_ARB_shader_storage_buffer_object :     enable

uniform vec4 		objectColour;
layout(r8ui, binding = 0) uniform uimage2D maskTex;
layout (binding = 1) uniform sampler2D baseTex;
layout (binding = 2) uniform sampler2DShadow shadowTex;
layout (binding = 3) uniform sampler2D bumpTex;
layout (binding = 4) uniform sampler2D metallicTex;
layout (binding = 5) uniform sampler2D roughnessTex;
layout (binding = 7) uniform sampler2D emissionTex;
layout (binding = 8) uniform sampler2D AOTex;
layout (binding = 9) uniform sampler2D opacityTex;
layout (binding = 10) uniform sampler2D glossTex;

uniform bool useBumpMap;
uniform bool useMetallicMap;
uniform bool useRoughnessMap;
uniform bool useHeightMap;
uniform bool useEmissionMap;
uniform bool useAOMap;
uniform bool useOpacityMap;
uniform bool useGlossMap;

uniform float timePassed;
uniform float timeScale;



uniform vec3	lightPos;
uniform float	lightRadius;
uniform vec4	lightColour;

uniform vec3	cameraPos;

uniform bool hasTexture;

uniform bool isPaintable;

uniform int width;
uniform int height;

uniform float noiseScale;
uniform float noiseOffsetSize;
uniform float noiseNormalStrength;
uniform float noiseNormalNoiseMult;

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
	vec3 tangent;
	vec3 binormal;
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

void point(inout vec4 finalColor, vec4 diffuse, vec3 bumpNormal, float metal, float rough, float reflectivity) {
	vec3 lightDir = normalize(lightPos - IN.worldPos);
	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	vec3 halfDir = normalize(lightDir + viewDir);

	float normalDotLightDir = max(dot(bumpNormal, lightDir), 0.0001);
	float normalDotViewDir = max(dot(bumpNormal, viewDir), 0.0001);
	float normalDotHalfDir = max(dot(bumpNormal, halfDir), 0.0001);
	float halfDirDotViewDir = max(dot(halfDir, viewDir), 0.0001);

	float F = reflectivity + (1 - reflectivity) * pow((1-halfDirDotViewDir), 5);

	F = 1 - F;
	


	float D = pow(rough, 2) / (3.14 * pow((pow(normalDotHalfDir, 2) * (pow(rough, 2) - 1) + 1), 2));

	

	float k = pow(rough + 1, 2) / 8;
	float G = normalDotViewDir / (normalDotViewDir * (1 - k) + k);

	float DFG = D * F * G;

	vec4 surface = diffuse * vec4(lightColour.rgb, 1) * lightColour.a;
	vec4 C = surface * (1 - metal);

	vec4 BRDF = ((1 - F) * (C / 3.14)) + (DFG / (4 * normalDotLightDir * normalDotViewDir));

	float dist = length(lightPos - IN.worldPos);
	float atten = 1.0 - clamp(dist / lightRadius , 0.0, 1.0);

	finalColor += BRDF * normalDotLightDir * atten;
	//if(isnan(BRDF.x) || isnan(BRDF.y) || isnan(BRDF.z))finalColor = vec4(1,0,0,0);
	//else finalColor = vec4(0,1,0,0);
	finalColor.a = 1;
}

//Nosie taken from: https://github.com/ashima/webgl-noise/blob/master/src/noise2D.glsl
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+10.0)*x);
}

float snoise(vec2 v)
  {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

vec2 bloodnoise(float scale) {
	vec2 tex = IN.texCoord;
	vec2 offset = vec2(snoise(tex * scale), 0.0);
	offset.y += snoise((tex + vec2(0.18468, 0.7846984) + timePassed * timeScale) * scale - 0.1 * offset.x);
	offset = 2.0 * offset - 1.0;
	return offset;
}

vec3 sampleTeamColor(vec2 uv) {
	vec2 realCoords = imageSize(maskTex) * uv;
	vec2 iCoords = floor(realCoords);
	vec2 fCoords = fract(realCoords);
	
	uint value00 = imageLoad(maskTex, ivec2(iCoords) + ivec2(0, 0)).r;
	uint value10 = imageLoad(maskTex, ivec2(iCoords) + ivec2(1, 0)).r;
	uint value01 = imageLoad(maskTex, ivec2(iCoords) + ivec2(0, 1)).r;
	uint value11 = imageLoad(maskTex, ivec2(iCoords) + ivec2(1, 1)).r;

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//fragColor = vec4(vec3(value00),1);
	//return;
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	vec3 a = teamColor(value00);
    vec3 b = teamColor(value10);
    vec3 c = teamColor(value01);
    vec3 d = teamColor(value11);

    vec2 u = fCoords;// * fCoords * (3.0 - 2.0 * fCoords);

    vec3 color = mix(a, b, u.x) +
        (c - a) * u.y * (1.0 - u.x) +
        (d - b) * u.x * u.y;
		
	return color;
}

void main(void)
{
	vec4 diffuse = texture(baseTex,IN.texCoord);

	int dataIndex = int(floor(IN.texCoord.y * height)) * width + int(floor(IN.texCoord.x * width));
	
	
	float shadow = 1.0; // New !
	
	if( IN . shadowProj. w > 0.0) { // New !
		shadow = textureProj ( shadowTex , IN . shadowProj );
	}
	shadow = max(shadow,0.2);

	mat3 TBN = mat3(normalize(IN.tangent),normalize(IN.binormal),normalize(IN.normal));

	vec3 bumpNormal = useBumpMap ? (2.0 * texture(bumpTex,IN.texCoord).rgb - 1.0) : IN.normal;
	
	{
		

		vec2 tex = IN.texCoord;

		vec2 offset = bloodnoise(noiseScale);
		vec2 normalOffset = bloodnoise(noiseNormalNoiseMult * noiseScale);

		vec3 maskAlbedo = sampleTeamColor(tex + offset * noiseOffsetSize);

		vec3 normalSkew = noiseNormalStrength * vec3(normalOffset.x, 0, normalOffset.y);
		normalSkew = TBN * normalSkew.xzy;
		bumpNormal = TBN * bumpNormal;

		float opacity = min(maskAlbedo.r + maskAlbedo.g + maskAlbedo.b, 1);
		opacity = smoothstep(0.4, 0.5, opacity);

		diffuse.rgb = mix(diffuse.rgb, maskAlbedo, opacity);
		bumpNormal = mix(bumpNormal, bumpNormal + normalSkew, opacity);
		bumpNormal = normalize(bumpNormal);

	}


/////////////////////////////////////////////////////////////////////////////////////////////////


	




	vec4 metallic = useMetallicMap ? texture(metallicTex,IN.texCoord) : vec4(0);
	vec4 roughness = useRoughnessMap ? texture(roughnessTex,IN.texCoord) : vec4(0);
	vec4 emission = useEmissionMap ? texture(emissionTex, IN.texCoord) : vec4(0);
	float AO = useAOMap ? texture(AOTex,IN.texCoord).r : 1;
	float opacity = useOpacityMap ? texture(opacityTex,IN.texCoord).r : 1;
	float reflectivity = useGlossMap ? texture(glossTex,IN.texCoord).r : 0.8;

	fragColor = vec4(0,0,0,1);
	point(fragColor,diffuse,bumpNormal,metallic.r,roughness.r,reflectivity);
	fragColor.rgb *= shadow;
	fragColor += diffuse * 0.5 * pow(AO,2);
	//fragColor.a = opacity;//todo add back in
	fragColor += emission;
	//fragColor = vec4(bumpNormal,1);
	//fragColor = vec4(bumpNormal,1);
}

