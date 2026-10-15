#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

vec4 gammaToLinear(vec4 col) {
    col.rgb = pow(col.rgb, vec3(2.2));
    return col;
}

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    texelColor = gammaToLinear(texelColor);
    finalColor = texelColor*gammaToLinear(colDiffuse)*fragColor;
}