#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in float color;
layout(location = 2) in int highlight;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;  // Ref of the eye camera (offset)
uniform mat4 modelMatrix;
uniform bool selecting;
uniform vec4 cursor; // In world space coordinates
uniform mat4 cameraRef; // Ref of the single camera (center of the head)

out float v_Color;
flat out int vHighlight;
out float cursorDistance;
out float depthFromCursorPlane;

void main() {
    vec4 world_position = modelMatrix * position;
    gl_Position = projMatrix * viewMatrix * world_position;

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

    vec4 headcoords = cameraRef * world_position;

    depthFromCursorPlane = headcoords.z - (cameraRef*cursor).z;

}
