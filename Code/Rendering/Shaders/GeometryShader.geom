#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

void main(){

	oColor = vec4(0,0,1,1);

    vec3 c = gl_in[0].gl_Position.xyz;

    gl_Position = c + (-1.0f,-1.0f); gTexCoord = vec2(0,0); EmitVertex();
    gl_Position = c + (-1.0f,1.0f); gTexCoord = vec2(0,1); EmitVertex();
    gl_Position = c + (1.0f,1.0f); gTexCoord = vec2(1,0); EmitVertex();
    gl_Position = c + (1.0f,-1.0f); gTexCoord = vec2(1,1); EmitVertex();
}
