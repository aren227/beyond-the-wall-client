#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

vec3 colorFilter(vec3 x) {
    float luminance = (max(max(x.r, x.g), x.b) + min(min(x.r, x.g), x.b)) * 0.5;
    float shadowWeight = 1.0 - smoothstep(0.0, 0.5, luminance);
    return clamp(x + vec3(-0.05, 0.0, 0.05) * shadowWeight, 0.0, 1.0);
}

void main()
{
    vec4 col = texture2D(texture0, fragTexCoord);
    col.rgb = ACESFilm(col.rgb);
    col.rgb = pow(max(col.rgb, 0.0), vec3(1.0 / 2.2));
    col.rgb = colorFilter(col.rgb);

    gl_FragColor = col;
}