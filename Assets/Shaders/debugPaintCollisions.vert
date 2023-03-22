#version 400 core
#extension GL_ARB_shader_storage_buffer_object : require

layout(binding=0, std430) buffer Positions {
    float positions[];
};

layout(binding=1, std430) buffer Indices {
    uint indices[];
};

layout(binding=2, std430) buffer TriangleIDs {
    uint triangleIds[];
};

uniform mat4 mvpMatrix;
uniform vec4 color;

out vec4 outColor;

vec3 position(uint index) {
	return vec3(positions[3 * index + 0], positions[3 * index + 1], positions[3 * index + 2]);
}

void main(void)
{
	uint id = gl_VertexID / 3;
	uint subId = gl_VertexID % 3;
	uint index = indices[3 * triangleIds[id] + subId];

	gl_Position = mvpMatrix * vec4(position(index), 1.0);
	
	outColor = color;
}