#version 330 core

uniform bool hasColors;
uniform bool isCursor;

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
		// good for color scalars
    //}
	
	if (hasColors && v_Color >= 0.0)// If this specific point belongs to a cluster (v_Color >= 0)
    {	
		vec3 color = texture(colormap, vec2(v_Color, 0.5)).rgb;
        fragColor = vec4(color, 0.3);
		//good for color cluster data
    }
	
    else
        fragColor = vec4(1, 1, 1, 0.3/255);
}
