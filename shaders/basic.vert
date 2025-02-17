#version 410 core

vec2 position[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

vec2 texCoord[3] = vec2[](
    vec2(0.0, 0.0),
    vec2(2.0, 0.0),
    vec2(0.0, 2.0)
);

out vec2 outTexCoord;

void main() {
    gl_Position = vec4(position[gl_VertexID], 0.0, 1.0);
    outTexCoord = texCoord[gl_VertexID]; 
}