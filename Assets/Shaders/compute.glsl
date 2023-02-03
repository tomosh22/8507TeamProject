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


#version 420 core
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

void main() {
	for (int s = leftS; s < rightS; s++)
	{
		for (int t = topT; t < bottomT; t++)
		{
            ivec2 IMAGE_COORD = ivec2(s, t);
            if (length(IMAGE_COORD - center) < radius) {
                imageStore(tex, IMAGE_COORD, vec4(1, 0, 0, 1));
            }
			
			
		}
	}

}