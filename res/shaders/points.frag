#version 330 core

uniform bool hasColors;

uniform sampler2D colormap;

in float v_Color;

out vec4 fragColor;

void main()
{
    if (hasColors)
    {
        vec3 color = texture(colormap, vec2(v_Color, 1 - v_Color)).rgb;
        fragColor = vec4(color, v_Color*2);// + vec4(1, 1, 1, 0.01);
    }
    else
        fragColor = vec4(1, 1, 1, 0.3/255);
}
