#version 410 core

in vec2 outTexCoord;
layout(location = 0) out vec4 FragColor;

uniform sampler2D computedTexture;

void main() {
    FragColor = texture(computedTexture, outTexCoord);
}

