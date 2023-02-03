//#version 420 core
//layout(local_size_x=1,local_size_y=1,local_size_z=1) in;
//layout (r8, binding = 0) uniform image2D screen;
//
//uniform int start;
//uniform int end;
//
//void main(){
//	int currentIndex = start;
//	for (int x = 0; x < 5000; x++)
//	{
//		for (int y = 0; y < 5000; y++)
//		{
//			if (start > end) { return; }
//			ivec2 IMAGE_COORD = ivec2(x,y);
//			imageStore(screen, IMAGE_COORD, vec4(1,0,0,1));
//		}
//		
//	}
//}


//#version 420 core
//layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
//layout(r8, binding = 0) uniform image2D tex;
//
//uniform int start;
//uniform int end;
//uniform int width;
//uniform int height;
//
//uniform int leftS;
//uniform int rightS;
//uniform int topT;
//uniform int bottomT;
//
//void main() {
//	int currentIndex = start;
//	while (currentIndex <= end) {
//		ivec2 IMAGE_COORD = ivec2(mod(currentIndex,width), floor(currentIndex / width));
//		imageStore(tex, IMAGE_COORD, vec4(1, 0, 0, 1));
//		currentIndex++;
//	}
//
//}


#version 430 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(r8, binding = 0) uniform image2D tex;


uniform int width;
uniform int height;

uniform int radius;

uniform int leftS;
uniform int rightS;
uniform int topT;
uniform int bottomT;

uniform ivec2 center;

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

void main() {
//	for (int s = leftS; s < rightS; s++)
//{
//	for (int t = topT; t < bottomT; t++)
//	{
//		ivec2 IMAGE_COORD = ivec2(s, t);
//		if (length(IMAGE_COORD - center) < radius - noise(IMAGE_COORD / (radius / 10)) * radius / 5) {
//			imageStore(tex, IMAGE_COORD, vec4(1, 0, 0, 1));
//		}
//
//
//	}
//}

        ivec2 IMAGE_COORD = ivec2(leftS + gl_GlobalInvocationID.x, topT + gl_GlobalInvocationID.y);
        vec2 noiseVec = vec2(IMAGE_COORD.x,IMAGE_COORD.y)/(radius/6);
		if (length(IMAGE_COORD - center) < radius + cnoise(noiseVec)*radius/5) {
			imageStore(tex, IMAGE_COORD, vec4(1, 0, 0, 1));
		}

}