#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

out vec2 center;
out float oRadius;

uniform float u_radius;

in vec3 Color[];
out vec3 oColor;

void main(){
	oColor = Color[0];
	//gl_Pos is from -1 to 1 and FragmentShader from 0 to 1 so circle center Pos and radius have to be scaled accordingly
	center = (vec2(gl_in[0].gl_Position) + vec2(1f)) * 0.5f;
	oRadius = 0.5f * u_radius;

	gl_Position = gl_in[0].gl_Position + vec4(-u_radius,-u_radius,0.0f,0.0f);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(-u_radius,u_radius,0.0f,0.0f);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(u_radius,-u_radius,0.0f,0.0f);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(u_radius,u_radius,0.0f,0.0f);
	EmitVertex();

	EndPrimitive();
}
