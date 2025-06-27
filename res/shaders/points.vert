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
out float depthFromCursorPlane;

void main() {
    vec4 world_position = modelMatrix * position;
    vec4 view_position = viewMatrix * world_position;
    gl_Position = projMatrix * view_position;

    gl_PointSize = 1 + 3 / (gl_Position.w+1);
    //gl_PointSize = 1;
    
    v_Color = color;
    vHighlight = highlight;
    cursorDistance = 0.0;

    if(selecting){
        cursorDistance = sqrt(
            pow(cursor.x - world_position.x, 2)
            + pow(cursor.y - world_position.y, 2)
            + pow(cursor.z - world_position.z, 2)
        );
    }
    vec4 cursor4 = vec4(cursor, 1.0);

    depthFromCursorPlane = view_position.z - (viewMatrix*cursor4).z;
}
