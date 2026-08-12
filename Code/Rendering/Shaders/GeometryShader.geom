#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

out vec2 center;
out vec2 oRadius;

uniform float u_radius;
uniform vec2 u_resolution;
uniform bool u_render_complex;
uniform float u_radius_increase;

uniform float u_pixels_per_meter;
   

in vec3 Color[];
out vec3 oColor;
out vec2 gTexCoord;

//out vec2 resolution;

void main(){

	oColor = Color[0];
	//*2 => Same as Vertex shader
	vec2 radius = vec2(u_radius* u_pixels_per_meter*2,u_radius * u_pixels_per_meter * 2)/u_resolution;
	if(u_render_complex){
		radius *= u_radius_increase;
	}

	gl_Position = gl_in[0].gl_Position + vec4(radius * vec2(-1.0f,-1.0f),0,0);
	gTexCoord = vec2(0f,0f);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(radius * vec2(-1.0f,1.0f),0,0);
	gTexCoord = vec2(0,1.0f);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(radius * vec2(1.0f,-1.0f),0,0);
	gTexCoord = vec2(1.0f,0);
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(radius * vec2(1.0f,1.0f),0,0);
	gTexCoord = vec2(1.0f,1.0f);
	EmitVertex();
}
