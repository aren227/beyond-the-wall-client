#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

out vec2 fragTexCoord;
out vec4 fragColor;

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