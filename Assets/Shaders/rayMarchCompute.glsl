

#version 430 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(r8, binding = 0) uniform image2D tex;





void main() {

		ivec2 IMAGE_COORD = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
		imageStore(tex,IMAGE_COORD,vec4(1,0,0,1));
		return;

}