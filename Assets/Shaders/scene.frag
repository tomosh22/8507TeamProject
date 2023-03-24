#version 430 core
#extension GL_ARB_shader_storage_buffer_object :     enable

uniform vec4 		objectColour;
layout (/*binding = 0,*/ r8ui) uniform uimage2D maskTex;
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

uniform bool toneMap = false;
uniform float exposure = 1;

uniform bool useTriplanarMapping;

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

	float c = fract(float(v + 8) * 3.14159265359);
	return hsv2rgb(vec3(c, 0.9, 1.0));
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

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r) {
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v) { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.5 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 105.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
}

vec2 bloodnoise(float scale) {
	vec3 tex = IN.worldPos;
	vec2 offset = vec2(snoise(tex * scale), 0.0);
	offset.y += snoise((tex + vec3(650.46845, 86.5651, -154.98456) + timePassed * timeScale) * scale - 0.1 * offset.x);
	offset = 2.0 * offset - 1.0;
	return offset;
}

vec3 fetchTeamColor(ivec2 coord, ivec2 size) {
	coord = clamp(coord, ivec2(0), size - ivec2(1));

	return teamColor(imageLoad(maskTex, coord).r);
}

vec3 sampleTeamColor(vec2 uv) {
	ivec2 maskSize = imageSize(maskTex);
	vec2 realCoords = maskSize * uv - vec2(0.5);
	vec2 iCoords = floor(realCoords);
	vec2 fCoords = fract(realCoords);
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//fragColor = vec4(vec3(value00),1);
	//return;
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	vec3 a = fetchTeamColor(ivec2(iCoords) + ivec2(0, 0), maskSize);
    vec3 b = fetchTeamColor(ivec2(iCoords) + ivec2(1, 0), maskSize);
    vec3 c = fetchTeamColor(ivec2(iCoords) + ivec2(0, 1), maskSize);
    vec3 d = fetchTeamColor(ivec2(iCoords) + ivec2(1, 1), maskSize);

    vec2 u = fCoords;// * fCoords * (3.0 - 2.0 * fCoords);

    vec3 color = mix(a, b, u.x) +
        (c - a) * u.y * (1.0 - u.x) +
        (d - b) * u.x * u.y;
		
	return color;
}

vec3 triplanarSample(sampler2D sampler){
	if(!useTriplanarMapping) return texture(sampler,IN.texCoord).rgb;
	vec3 xy = texture(sampler, IN.worldPos.xy * .1).rgb;//todo uniform
	vec3 xz = texture(sampler, IN.worldPos.xz * .1).rgb;
	vec3 yz = texture(sampler, IN.worldPos.yz * .1).rgb;

	vec3 myNormal = abs(IN.normal);
	myNormal = pow(myNormal, vec3(1));//todo uniform
	myNormal /= myNormal.x + myNormal.y + myNormal.z;
	vec3 result = xz*myNormal.y + xy*myNormal.z + yz*myNormal.x;
	return result;
}

vec4 triplanarSampleVec4(sampler2D sampler){
	if(!useTriplanarMapping) return texture(sampler,IN.texCoord);
	vec4 xy = texture(sampler, IN.worldPos.xy * .1);//todo uniform
	vec4 xz = texture(sampler, IN.worldPos.xz * .1);
	vec4 yz = texture(sampler, IN.worldPos.yz * .1);

	vec3 myNormal = abs(IN.normal);
	myNormal = pow(myNormal, vec3(1));//todo uniform
	myNormal /= myNormal.x + myNormal.y + myNormal.z;
	vec4 result = xz*myNormal.y + xy*myNormal.z + yz*myNormal.x;
	return result;
}

void main(void)
{
	vec4 diffuse = triplanarSampleVec4(baseTex);
	diffuse.rgb = pow(diffuse.rgb, vec3(2.2));

	float shadow = 1.0; // New !
	
	if( IN . shadowProj. w > 0.0) { // New !
		shadow = textureProj ( shadowTex , IN . shadowProj );
	}
	shadow = max(shadow,0.2);

	mat3 TBN = mat3(normalize(IN.tangent),normalize(IN.binormal),normalize(IN.normal));

	vec3 bumpNormal = useBumpMap ? (2.0 * triplanarSample(bumpTex) - 1.0) : IN.normal;
	
	//PBR stuff
	vec3 metallic = useMetallicMap ? triplanarSample(metallicTex) : vec3(0);
	vec3 roughness = useRoughnessMap ? triplanarSample(roughnessTex) : vec3(0);
	vec3 emission = useEmissionMap ? triplanarSample(emissionTex) : vec3(0);
	float AO = useAOMap ? triplanarSample(AOTex).r : 1;
	float opacity = useOpacityMap ? triplanarSample(opacityTex).r : 1;
	float reflectivity = useGlossMap ? triplanarSample(glossTex).r : 0.8;

	emission = pow(emission, vec3(2.2));

	vec4 baseColor = vec4(0,0,0,1);
	point(baseColor,diffuse,bumpNormal,metallic.r,roughness.r,reflectivity);
	baseColor.rgb += diffuse.rgb * 0.5 * pow(AO,2);
	baseColor.rgb *= shadow;
	baseColor.rgb += emission * 100;
	baseColor.a = 1;

	//Paint stuff
	{
		vec2 offset = bloodnoise(noiseScale);
		vec2 normalOffset = bloodnoise(noiseNormalNoiseMult * noiseScale);

		vec3 maskAlbedo = sampleTeamColor(IN.texCoord + offset * noiseOffsetSize);

		vec3 normalSkew = noiseNormalStrength * vec3(normalOffset.x, 0, normalOffset.y);
		normalSkew = TBN * normalSkew.xzy;
		bumpNormal = TBN * bumpNormal;

		float opacity = min(maskAlbedo.r + maskAlbedo.g + maskAlbedo.b, 1);
		opacity = smoothstep(0.4, 0.5, opacity);
		
		bumpNormal = normalize(bumpNormal + normalSkew);

		vec4 paintColor = vec4(maskAlbedo, 1.0);
		point(paintColor,paintColor,normalize(bumpNormal),0,0.1,0.5);
		paintColor *= shadow;

		fragColor.rgb = mix(baseColor.rgb, paintColor.rgb, opacity);
		fragColor.a = 1;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////

	//fragColor.a = opacity;//todo add back in
	//fragColor = vec4(bumpNormal,1);
	//fragColor = vec4(bumpNormal,1);

	if(toneMap){
		const float gamma = 2.2;
		vec3 hdrColor = fragColor.rgb;
  
		// exposure tone mapping
		vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
		// gamma correction 
		//mapped = pow(mapped, vec3(1.0 / gamma));
  
		fragColor = vec4(mapped, 1.0);
	}
}

