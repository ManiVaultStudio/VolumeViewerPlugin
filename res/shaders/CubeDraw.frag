#version 330 core

in vec3 pass_position;
in vec3 pass_normal;

out vec4 fragColor;

void main()
{    
	vec3 lightDir = normalize(vec3(-0.5, -1, -0.5));
	float light = max(0, dot(-lightDir, pass_normal)) + 0.3;
	vec3 color = (pass_normal * 0.5 + 0.5) * light;
    fragColor = vec4(color, 1);
}
