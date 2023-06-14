#version 450 core
layout (location = 0) in vec2 aPos;
out vec3 outPos;

void main()
{
	gl_PointSize = 3;
	gl_Position = vec4(aPos, 1.0, 1.0);
	outPos = vec3(aPos, gl_VertexID);
}