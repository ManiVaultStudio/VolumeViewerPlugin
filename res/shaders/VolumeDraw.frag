#version 330 core

uniform bool hasColors;
uniform bool isCursor;
uniform vec3 selectionColor;
uniform bool live; // false when tracking is lost

uniform sampler2D colormap;
uniform bool distanceEffect;

in float v_Color;
flat in int vHighlight;
in float cursorDistance;

out vec4 fragColor;

void main()
{

    fragColor = vec4(1, 1, 1, .6);

    if (hasColors) {
        vec3 color = texture(colormap, vec2(v_Color, 0)).rgb;
        fragColor = vec4(color, 1);
    }

    if (isCursor || vHighlight != 0)
    {
        fragColor = vec4(selectionColor, 1);
    }

    if(distanceEffect){
        // Remove the color around the cursor
        // fragColor = mix(vec4(0.0,0.0,0.0,0.7), fragColor, min(1.0, max(0.0, cursorDistance*6)));
        // Add a drop of selection color gradient close to the cursor to highlight proximity
        fragColor = mix(vec4(selectionColor,0.8), fragColor, min(1.0, max(0.0, cursorDistance*5)));
        // For very close points, display strong selection color
        if(cursorDistance < 0.02){
            fragColor = vec4(selectionColor,1);
        }
    }
    
    // If the position is not tracked live, display the data in dark gray
    if(!live){
        fragColor = mix(vec4(0.7,0.7,0.7,0.3), fragColor, 0.4);
    }
}