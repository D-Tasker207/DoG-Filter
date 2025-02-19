#version 410 core

in vec2 outTexCoord;

uniform sampler2D imageSampler;

out vec4 fragColor;

void main() {
    vec4 color = texture(imageSampler, outTexCoord);

    float grey = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;

    fragColor = vec4(grey, grey, grey, 1.0);
}