#version 410 core

in vec2 outTexCoord;
out vec4 fragColor;

uniform sampler2D originalTexture;
uniform sampler2D computedTexture;

void main() {
    fragColor = texture(computedTexture, outTexCoord);
    // fragColor = vec4(outTexCoord, 0.0, 1.0);
}
