#version 410 core

in vec2 outTexCoord;

uniform sampler2D etfSampler;
uniform int kernelSize;
uniform float sigma;

out vec4 fragColor;

float gaussian1d(float x, float sigma) {
    return exp(-x * x / (2.0 * sigma * sigma)) / (sqrt(2.0 * 3.14159265359) * sigma);
}

void main() {
    vec2 texelSize = 1.0 / textureSize(etfSampler, 0);

    vec2 sum = vec2(0.0);
    float weightSum = 0.0;
    for(int j = -kernelSize; j <= kernelSize; j++) {
        for(int i = -kernelSize; i <= kernelSize; i++) {
            vec2 offset = vec2(float(i), float(j)) * texelSize;

            float weight = gaussian1d(float(i), sigma) * gaussian1d(float(j), sigma);

            vec2 tangent = texture(etfSampler, outTexCoord + offset).xy;
            sum += tangent * weight;
            weightSum += weight;
        }
    }

    vec2 result = sum / weightSum;

    fragColor = vec4(result, 0.0, 1.0);
}


