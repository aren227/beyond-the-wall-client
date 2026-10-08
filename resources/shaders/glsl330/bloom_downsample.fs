#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec4 textureSize;

// Output fragment color
out vec4 finalColor;

void main()
{
    vec4 sum = vec4(0);

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            sum += texture(texture0, fragTexCoord + textureSize.zw * vec2(x, y));
        }
    }

    finalColor = sum / 9.0 * colDiffuse * fragColor;
}