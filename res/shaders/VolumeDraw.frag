#version 330 core

uniform bool hasColors;
uniform bool isCursor;
uniform vec3 selectionColor;

uniform sampler2D colormap;
uniform vec2 mapSize;

in float v_Color;
flat in int vHighlight;

out vec4 fragColor;

void main()
{

    fragColor = vec4(1, 1, 1, .6);

    if (hasColors) {
        vec3 color = texture(colormap, vec2(v_Color/mapSize.x, 0)).rgb;
        fragColor = vec4(color, 1);
    }

    if (isCursor || vHighlight != 0)
    {
        fragColor = vec4(selectionColor, 1);
    }
    
}