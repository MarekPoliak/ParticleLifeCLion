#version 450
layout(local_size_x = 1, local_size_y = 1, local_size_y = 1) in;

layout(std430, binding = 0) buffer VerticesBuffer
{
    float vertices[];
};

layout(std430, binding = 1) buffer VectorsBuffer
{
    float vectors[];
};

layout(std430, binding = 2) buffer input_layout
{
    vec4 outData[];
};

uniform vec2 random;
uniform int colorCount;
uniform vec3 colors[6];
uniform float colorMatrix[6][6];
uniform float colorDropOffIn[6];
uniform float colorDropOffOut[6];

float rand(vec2 co);
float magnitude(float a, float b);

void main() {
    float x = vertices[gl_GlobalInvocationID.x * 2];
    float y = vertices[gl_GlobalInvocationID.x * 2 + 1];
    float vecx = vectors[gl_GlobalInvocationID.x * 2];
    float vecy = vectors[gl_GlobalInvocationID.x * 2 + 1];

    float xAdd = random.x;
    float yAdd = random.y;
    vecx = vecx + xAdd;
    vecy = vecy + yAdd;
    float mag = magnitude(vecx, vecy);
    if (mag > 0.005f) {
        float divisor = mag / 0.005f;
        vecx = vecx / divisor;
        vecy = vecy / divisor;
    }

    x = mod(x + vecx + 1.0, 2.0) - 1.0;
    y = mod(y + vecy + 1.0, 2.0) - 1.0;
    outData[gl_GlobalInvocationID.x] = vec4(x,y,vecx,vecy);
}

float magnitude(float a, float b) {
    return sqrt(a * a + b * b);
}