#version 330 core

uniform bool hasColors;
uniform bool isCursor;
uniform bool useOpacityModulation;

uniform sampler2D colormap;

in float v_Color;

out vec4 fragColor;

void main()
{
    if (isCursor)
    {
        fragColor = vec4(1, 0, 0, 1);
        return;
    }
	
    //if (hasColors)
    //{
        //vec3 color = texture(colormap, vec2(v_Color, 1 - v_Color)).rgb;
        //fragColor = vec4(color, min(0.3, max(v_Color*6-2.5, 0))); //fade out pt with low scalar values
		// good for color point data - Color and Opacity points by scalars
    //}
	
	if (hasColors && v_Color >= 0.0) 
   {    vec3 color = texture(colormap, vec2(v_Color, 0.5)).rgb;
        if (useOpacityModulation) {       
            fragColor = vec4(color, min(0.3, max(v_Color * 6.0 - 2.5, 0.0))); // fade out pt with low scalar values
        } else 
		{    
            fragColor = vec4(color, 0.3);//for cluster data
        }
    }
	
    else
        fragColor = vec4(1, 1, 1, 0.3/255);
		
	
}
