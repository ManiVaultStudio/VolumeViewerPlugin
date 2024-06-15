// SPDX-License-Identifier: LGPL-3.0-or-later 
// A corresponding LICENSE file is located in the root directory of this source tree 
// Copyright (C) 2023 BioVault (Biomedical Visual Analytics Unit LUMC - TU Delft) 

#version 330 core

uniform sampler2D leftImage;
uniform sampler2D rightImage;

in vec2 pass_texCoord;

out vec4 fragColor;

void main() {
    if (int(gl_FragCoord.y) % 2 == 0)
        fragColor = texture(leftImage, pass_texCoord) * vec4(1, 0.3, 0.3, 1);
    else
        fragColor = texture(rightImage, pass_texCoord) * vec4(0.3, 0.3, 1, 1);
}
