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

        fragColor = vec4(color, min(0.3, max(v_Color*6-2.5, 0)));
    }
    else
        fragColor = vec4(1, 1, 1, 0.3/255);
}
