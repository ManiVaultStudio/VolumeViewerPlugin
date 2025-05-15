#version 330 core

uniform bool hasColors;
uniform bool isCursor;
uniform bool usesColorMap;
uniform vec3 selectionColor;

uniform sampler2D colormap;
uniform vec2 mapSize;

in float v_Color;
flat in int vHighlight;

out vec4 fragColor;

void main()
{
    vec3 defaultColor = vec3(1,1,1);
    if (isCursor || vHighlight != 0)
    {
        fragColor = vec4(1, 0, 0, 1);
    } else if (hasColors) {
        if(usesColorMap){
            vec3 color = texture(colormap, vec2(v_Color/mapSize.x, 0)).rgb;
            fragColor = vec4(color, 1);
        } else {
            // Means we are selecting
            // v_Color = 0. or 1.
            vec3 color = selectionColor*v_Color + defaultColor*(1-v_Color); 
            fragColor = vec4(color, 1);
        }
    }
    else
        fragColor = vec4(1, 1, 1, .6);
    
}