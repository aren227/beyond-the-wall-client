#version 100

precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec4 colDiffuseHDR;

vec4 gammaToLinear(vec4 col) {
    col.rgb = pow(col.rgb, vec3(2.2));
    return col;
}

void main()
{
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    texelColor = gammaToLinear(texelColor);
    gl_FragColor = texelColor*gammaToLinear(colDiffuse)*gammaToLinear(colDiffuseHDR)*fragColor;
}