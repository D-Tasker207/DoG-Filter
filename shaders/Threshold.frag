#version 410 core

in vec2 outTexCoord;

uniform sampler2D imageSampler;
uniform float phi;
uniform float epsilon;

out vec4 fragColor;

void main() {
    float color = texture(imageSampler, outTexCoord).r;

    float T = (color >= epsilon) ? 1.0 : 1.0 + tanh(phi * (color - epsilon));

    fragColor = vec4(T, T, T, 1.0);
}