#version 100

precision mediump float;

attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec4 vertexColor;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform mat4 mvp;

vec4 gammaToLinear(vec4 col) {
    col.rgb = pow(col.rgb, vec3(2.2));
    return col;
}

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = gammaToLinear(vertexColor);
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}