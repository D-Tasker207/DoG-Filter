#version 410 core

in vec2 outTexCoord;

uniform sampler2D imageSampler;
uniform sampler2D etfSampler;
uniform int kernelSteps;
uniform int stepSize;
uniform float sigma;

out vec4 fragColor;

float gaussian1d(float x, float sigma) {
    return exp(-x * x / (2.0 * sigma * sigma)) / (sqrt(2.0 * 3.14159265359) * sigma);
}

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(imageSampler, 0));

    vec3 sumColor = vec3(0.0);
    float weightSum = 0.0;

    float centerweight = gaussian1d(0.0, sigma);
    sumColor += texture(imageSampler, outTexCoord).rgb * centerweight;
    weightSum += centerweight;
    
    // integrating forwards along flow
    vec2 currentCoord = outTexCoord;
    float t = 0.0;
    for(int i = 1; i <= kernelSteps; i++){
        vec2 flow = normalize(texture(etfSampler, currentCoord).xy);
        currentCoord += flow * stepSize;
        t += stepSize;

        if(currentCoord.x < 0.0 || currentCoord.x > 1.0 || 
           currentCoord.y < 0.0 || currentCoord.y > 1.0) {
            break;
        }

        float w = gaussian1d(t, sigma);
        sumColor += texture(imageSampler, currentCoord).rgb * w;
        weightSum += w;
    }

    // integrating backwards along flow
    currentCoord = outTexCoord;
    t = 0.0;
    for(int i = 1; i <= kernelSteps; i++){
        vec2 flow = normalize(texture(etfSampler, currentCoord).xy);
        currentCoord -= flow * stepSize;
        t += stepSize;
        if (currentCoord.x < 0.0 || currentCoord.x > 1.0 || 
            currentCoord.y < 0.0 || currentCoord.y > 1.0) {
            break;
        }

        float w = gaussian1d(t, sigma);
        sumColor += texture(imageSampler, currentCoord).rgb * w;
        weightSum += w;
    }

    vec3 lic_result = sumColor / weightSum;

    fragColor = vec4(lic_result, 1.0);
}