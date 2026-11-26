#version 100

precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec4 misc;

vec4 gammaToLinear(vec4 col) {
    col.rgb = pow(col.rgb, vec3(2.2));
    return col;
}

// https://www.shadertoy.com/view/DsK3W1
vec2 n22 (vec2 p) {
    vec3 a = fract(p.xyx * vec3(123.34, 234.34, 345.65));
    a += dot(a, a + 34.45);
    return fract(vec2(a.x * a.y, a.y * a.z));
}

vec2 get_gradient(vec2 pos) {
    float twoPi = 6.283185;
    float angle = n22(pos).x * twoPi;
    return vec2(cos(angle), sin(angle));
}

float perlin_noise(vec2 uv) {
    vec2 pos_in_grid = uv;
    vec2 cell_pos_in_grid =  floor(pos_in_grid);
    vec2 local_pos_in_cell = (pos_in_grid - cell_pos_in_grid);
    vec2 blend = local_pos_in_cell * local_pos_in_cell * (3.0 - 2.0 * local_pos_in_cell);

    vec2 left_top = cell_pos_in_grid + vec2(0.0, 1.0);
    vec2 right_top = cell_pos_in_grid + vec2(1.0, 1.0);
    vec2 left_bottom = cell_pos_in_grid + vec2(0.0, 0.0);
    vec2 right_bottom = cell_pos_in_grid + vec2(1.0, 0.0);

    float left_top_dot = dot(pos_in_grid - left_top, get_gradient(left_top));
    float right_top_dot = dot(pos_in_grid - right_top,  get_gradient(right_top));
    float left_bottom_dot = dot(pos_in_grid - left_bottom, get_gradient(left_bottom));
    float right_bottom_dot = dot(pos_in_grid - right_bottom, get_gradient(right_bottom));

    float noise_value = mix(
                            mix(left_bottom_dot, right_bottom_dot, blend.x),
                            mix(left_top_dot, right_top_dot, blend.x),
                            blend.y);


    return (0.5 + 0.5 * (noise_value / 0.7));
}

vec4 sampleColor(vec2 uv) {
    float n = perlin_noise(uv * vec2(0.5, 1.0) + vec2(-misc.z, 0) * 10.0);
    n = n - 0.5;

    float d = clamp(1.0 - abs(uv.y - 0.5) / 0.5, 0.0, 1.0);

    if (n+d < 0.3) discard;

    float n2 = perlin_noise(uv * vec2(0.1, 4.0) + vec2(-misc.z, 0) * 20.0);
    if (n2 < 0.2) discard;

    return vec4(1.0);
    // return vec4(vec3(n2 > 0.5 ? 1.5 : 1.0), 1.0);
}

void main()
{
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    texelColor = gammaToLinear(texelColor);
    gl_FragColor = texelColor
        *gammaToLinear(colDiffuse)
        *gammaToLinear(fragColor) // XXX(aren): Do this in vert
        *sampleColor(fragTexCoord)
        *gammaToLinear(vec4(vec3(2.0), 1.0));
}
