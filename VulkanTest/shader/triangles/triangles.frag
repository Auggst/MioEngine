#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

mat4 srgb2P3Matrix = mat4(
    vec4(1.2249, -0.2247, 0, 0),
    vec4(-0.0420, 1.0419, 0, 0),
    vec4(-0.0197, -0.0786, 1.0979, 0),
    vec4(0, 0, 0, 1)
);

vec3 gammaEncoded(vec3 color);
vec3 gammaDecoded(vec3 color);
vec3 toSRGB(vec3 p3Color);

void main() {
    //outColor = vec4(toSRGB(fragColor), 1.0);
    outColor = vec4(fragColor, 1.0);
}

vec3 gammaEncoded(vec3 color) {
    vec3 newColor = vec3(1.0);
    for (int i = 0; i < 3; i++) {
        if (abs(color[i]) <= 0.0031308) {
            newColor[i] *= 12.92;
        } else {
            newColor[i] = sign(color[i]) * (pow(abs(color[i]), 1 / 2.4) * 1.055 - 0.055);
        }
    }
    return newColor;
}

vec3 gammaDecoded(vec3 color) {
    vec3 newColor = vec3(1.0);
    for (int i = 0; i < 3; i++) {
        if (abs(color[i]) <= 0.04045) {
            newColor[i] /= 12.92;
        } else {
            newColor[i] = sign(color[i]) * pow((abs(color[i]) + 0.055) / 1.055, 2.4);
        }
    }
    return newColor;
}

vec3 toSRGB(vec3 p3Color) {
    vec3 srgbColor = vec3(1.0);
    vec4 linearSrgb =vec4(gammaDecoded(p3Color), 1.0) *  srgb2P3Matrix;
    srgbColor = gammaEncoded(linearSrgb.xyz);
    srgbColor.x = clamp(srgbColor.x, 0.0, 1.0); 
    srgbColor.y = clamp(srgbColor.y, 0.0, 1.0);
    srgbColor.z = clamp(srgbColor.z, 0.0, 1.0);
    return srgbColor;
}