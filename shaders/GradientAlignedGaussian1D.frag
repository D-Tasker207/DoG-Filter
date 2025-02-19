#version 410 core

in vec2 outTexCoord;

uniform sampler2D imageSampler;
uniform sampler2D etfSampler;
uniform int kernelSize;
uniform float sigma;

out vec4 fragColor;

float gaussian1d(float x, float sigma) {
    return exp(-x * x / (2.0 * sigma * sigma)) / (sqrt(2.0 * 3.14159265359) * sigma);
}

void main() {
    vec2 tangent = normalize(texture(etfSampler, outTexCoord).xy);
    vec2 gradient = vec2(-tangent.y, tangent.x);
    vec2 texelsize = 1.0 / vec2(textureSize(imageSampler, 0));

    vec3 colorSum1 = vec3(0.0);
    float weightSum1 = 0.0;

    for(int i = -kernelSize; i <= kernelSize; i++){
        vec2 offset = gradient * float(i) * texelsize;
        vec3 sampleColor = texture(imageSampler, outTexCoord + offset).rgb;

        float w1 = gaussian1d(float(i), sigma);

        colorSum1 += sampleColor * w1;
        weightSum1 += w1;
    }

    vec3 blur1 = colorSum1 / weightSum1;

    fragColor = vec4(blur1, 1.0);
}