﻿#version 330 core
out vec4 FragColor;

in vec4 oColor;
in vec2 gTexCoord;

void main()
{
    
    vec2 uv = gTexCoord;
	if(distance(uv,vec2(0.5f,0.5f))>=0.5f) discard;
    float alpha = 1.0f;

    vec2 pos = (uv - 0.5) * 2.0;
    float r2 = dot(pos, pos);
    if (r2 > 1.0) discard;

    vec3 normal = vec3(pos, sqrt(1.0 - r2));

    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diffuse = max(dot(normal, lightDir), 0.0);
    float ambient = 0.15;

    FragColor = vec4(oColor.xyz * (ambient + 0.85 * diffuse), alpha);
}