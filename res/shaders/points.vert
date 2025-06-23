#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in float color;
layout(location = 2) in int highlight;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform bool selecting;
uniform vec3 cursor; // In world space coordinates

out float v_Color;
flat out int vHighlight;
out float cursorDistance;

void main() {
    vec4 world_position = modelMatrix * position;
    gl_Position = projMatrix * viewMatrix * world_position;

    gl_PointSize =  1.5 + 3 / gl_Position.w;

    
    v_Color = color;
    vHighlight = highlight;

    if(selecting){
        cursorDistance = sqrt(
            pow(cursor.x - world_position.x, 2)
            + pow(cursor.y - world_position.y, 2)
            + pow(cursor.z - world_position.z, 2)
        );
    }
}
