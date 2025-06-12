#version 330 core

uniform bool hasColors;
uniform bool isCursor;
uniform vec3 selectionColor;
uniform bool live; // false when tracking is lost
uniform int selectMode;
uniform float selectRadius;

uniform sampler2D colormap;
uniform bool selecting;

in float v_Color;
flat in int vHighlight;
in float cursorDistance;

out vec4 fragColor;

void main()
{

    fragColor = vec4(.6, .6, .6, 1);

    if (hasColors) {
        vec3 color = texture(colormap, vec2(v_Color, 1-v_Color)).rgb;
        fragColor = vec4(color, 1);
    }

    if(selecting){
        switch(selectMode){
            case 0: {
                // Nearest selection
                vec3 antiSelectionColor = vec3(1,1,1) - selectionColor;
                // General highligh in neighboroud
                fragColor.a = max(.5, 1-cursorDistance);
                // close-by coloured highlight
                fragColor = mix(vec4(antiSelectionColor,0.5), fragColor,  min(1, cursorDistance*6));
                // For very close points, display strong selection color
                if(cursorDistance < 0.03){
                    // Add a drop of selection color gradient close to the cursor to highlight proximity
                    fragColor = mix(vec4(antiSelectionColor, 1), fragColor, .5);
                }
                break;
            }
            case 1: {
                fragColor.a = 1;
                // Make the data outside the selection sphere slightly transparent
                if(cursorDistance > selectRadius){
                    float distanceFade = 0.3;
                    float minAlphaFade = 0.2;
                    if(cursorDistance < selectRadius + distanceFade){
                        fragColor.a = 0.7-(cursorDistance - selectRadius) * (0.7-minAlphaFade) / distanceFade;
                    }
                    else {
                        fragColor.a = minAlphaFade;
                    }
                }
                break;
            }
        }
    }

    // Highlight selected points
    if (isCursor || vHighlight != 0)
    {
        fragColor = vec4(selectionColor, 1);
    }
    
    // If the position is not tracked live, display the data in dark gray
    if(!live){
        fragColor = mix(vec4(0.7,0.7,0.7,0.3), fragColor, 0.4);
    }


}