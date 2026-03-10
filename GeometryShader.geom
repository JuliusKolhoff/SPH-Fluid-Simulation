#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

out vec2 center;
out vec2 oRadius;

uniform float u_radius;
uniform vec2 u_resolution;

in vec3 Color[];
out vec3 oColor;

out vec2 resolution;

void main(){
	oColor = Color[0];
	resolution = u_resolution;
	//gl_Pos is from -1 to 1 and FragmentShader from 0 to 1 so circle center Pos and radius have to be scaled accordingly
	center = (vec2(gl_in[0].gl_Position) + vec2(1f)) * 0.5f;
	//oRadius = 0.5f * (u_radius / 800);
	oRadius = vec2(u_radius) * 0.5f;

	gl_Position = gl_in[0].gl_Position + vec4(-u_radius/u_resolution.x,-u_radius/u_resolution.y,0.0f,0.0f);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(-u_radius/u_resolution.x,u_radius/u_resolution.y,0.0f,0.0f);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(u_radius/u_resolution.x,-u_radius/u_resolution.y,0.0f,0.0f);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(u_radius/u_resolution.x,u_radius/u_resolution.y,0.0f,0.0f);
	EmitVertex();

	EndPrimitive();
}
