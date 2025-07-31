#version 330 core

uniform bool hasColors;
uniform bool hasAlphas;
uniform bool isCursor;
uniform vec3 selectionColor;
uniform bool live; // false when tracking is lost
uniform int selectMode;
uniform float selectRadius;
uniform float baseOpacity;

uniform bool showingFlashlight;
uniform float flashlightSlope;
uniform float flashlightMin;
uniform bool isFlashlightSource;

uniform sampler2D colormap;
uniform bool selecting;

in float v_Color;
flat in int vHighlight;
in float cursorDistance;
in float depthFromCursorPlane;
flat in int shouldClip;
in float alpha;

out vec4 fragColor;

void main()
{
    float opacity = 0.0;
    if(hasAlphas) {
        if(showingFlashlight){
            opacity = max(1+alpha*flashlightSlope, flashlightMin);
        }else{
            opacity = alpha;
        }
    } else {
        opacity = baseOpacity;
    }
        fragColor = vec4(.6, .6, .6, opacity);

        if (hasColors) {
            vec3 color = texture(colormap, vec2(v_Color, 1-v_Color)).rgb;
        
            fragColor = vec4(color, opacity);
      

            //fragColor = vec4(v_Color, 0, 1-v_Color, 1); // Test color for Depth sorting test 
        }

        if(selecting){
            if(isFlashlightSource){
                opacity = max(1+alpha*flashlightSlope, flashlightMin);
            }
            else {
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
                        if(depthFromCursorPlane > 0){
                            fragColor.a = max(0.008,1-(depthFromCursorPlane)*20);
                        }
                        break;
                    }
                    case 1: {
                        fragColor.a = 1;
                        // Make the data outside the selection sphere slightly transparent
                        if(cursorDistance > selectRadius){
                            float distanceFade = 0.3;
                            float minAlphaFade = min(0.05, baseOpacity);
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
        }

        if (isCursor){
            fragColor = vec4(selectionColor, 1);
        }

        // Highlight selected points
        if (vHighlight != 0)
        {
            if(baseOpacity > 0.4){
                fragColor = vec4(selectionColor, 1);
            } else {
                fragColor.a = 1;
            }
        }
        
        // If the position is not tracked live, display the data in dark gray
        if(!live){
            fragColor.a *= 0.8; // mix(vec4(0.7,0.7,0.7,0.3), fragColor, 0.8);
        }
    
        
}