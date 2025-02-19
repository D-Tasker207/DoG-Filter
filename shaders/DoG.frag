#version 410 core

in vec2 outTexCoord;

uniform sampler2D blurredImage1;
uniform sampler2D blurredImage2;
uniform float tau;

out vec4 fragColor;

void main() {
    vec3 blurred1 = texture(blurredImage1, outTexCoord).rgb;
    vec3 blurred2 = texture(blurredImage2, outTexCoord).rgb;

    vec3 difference = (1 + tau) * blurred1 - tau * blurred2;

    fragColor = vec4(difference, 1.0);
}