#version 410 core

in vec2 outTexCoord;

uniform sampler2D imageSampler;

out vec4 fragColor;

void main() {
    vec2 texelSize = 1.0 / textureSize(imageSampler, 0);

    // Sample the surrounding pixels using the Sobel operator.
    // Sobel kernels for x and y directions.
    // Gx kernel:
    //  -1  0  1
    //  -2  0  2
    //  -1  0  1
    // Gy kernel:
    //  -1 -2 -1
    //   0  0  0
    //   1  2  1

    float lumTL = texture(imageSampler, outTexCoord + texelSize * vec2(-1, -1)).r;
    float lumTC = texture(imageSampler, outTexCoord + texelSize * vec2( 0, -1)).r;
    float lumTR = texture(imageSampler, outTexCoord + texelSize * vec2( 1, -1)).r;
    
    float lumML = texture(imageSampler, outTexCoord + texelSize * vec2(-1,  0)).r;
    float lumMC = texture(imageSampler, outTexCoord).r;
    float lumMR = texture(imageSampler, outTexCoord + texelSize * vec2( 1,  0)).r;
    
    float lumBL = texture(imageSampler, outTexCoord + texelSize * vec2(-1,  1)).r;
    float lumBR = texture(imageSampler, outTexCoord + texelSize * vec2( 1,  1)).r;
    float lumBC = texture(imageSampler, outTexCoord + texelSize * vec2( 0,  1)).r;

    // Apply the Sobel kernels
    float gradX = (-1.0 * lumTL) + (1.0 * lumTR) +
                  (-2.0 * lumML) + (2.0 * lumMR) +
                  (-1.0 * lumBL) + (1.0 * lumBR);
    float gradY = (-1.0 * lumTL) + (-2.0 * lumTC) + (-1.0 * lumTR) +
                  ( 1.0 * lumBL) + ( 2.0 * lumBC) + ( 1.0 * lumBR);

    // The gradient vector points in the direction of greatest intensity change.
    // For ETF, we want the edge tangent, which is perpendicular to the gradient.
    vec2 tangent = vec2(-gradY, gradX);
    
    // Normalize the tangent vector.
    float len = length(tangent);
    if (len > 0.0001)
        tangent = normalize(tangent);
    else
        tangent = vec2(0.0); 

    fragColor = vec4(tangent, 0.0, 1.0);
}