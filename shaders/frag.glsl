#version 450 core
in vec3 outPos;

layout(std430, binding = 3) buffer ColorIndicesBuffer
{
	int colorIndices[];
};

uniform vec3 colors[6];

void main()
{
	int particleIndex = int(outPos.z);
	int colorIndex= colorIndices[particleIndex];
	vec3 color = colors[colorIndex];
	gl_FragColor  = vec4(color, 1.0);
}