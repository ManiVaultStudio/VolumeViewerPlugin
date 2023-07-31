#version 330 core

uniform bool hasColors;

in float v_Color;

out vec4 fragColor;

void main()
{
    if (hasColors)
        fragColor = vec4(1, 0, 0, v_Color);// + vec4(1, 1, 1, 0.01);
    else
        fragColor = vec4(1, 1, 1, 0.01);
}
