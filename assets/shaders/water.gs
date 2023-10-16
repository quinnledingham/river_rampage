#version 420 core
layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

layout (std140) uniform Matrices
{
	mat4 projection;
	mat4 view;
};

in vec2 uv[];

out vec3 frag_normal;
out vec3 frag_tangent;
out vec3 frag_bitangent;
out vec3 frag_position;
out vec2 uv_coords;

void main(void)
{
	vec3 a = (gl_in[1].gl_Position - gl_in[0].gl_Position).xyz;
    vec3 b = (gl_in[2].gl_Position - gl_in[0].gl_Position).xyz;
    vec3 N = normalize(cross(b, a));

    for(int i = 0; i < gl_in.length(); i++)
    {
		uv_coords = uv[i];
		vec4 pos = gl_in[i].gl_Position;

		frag_position = pos.xyz;
        frag_normal = -N;
        gl_Position = projection * view * pos;
        EmitVertex();
    }

    EndPrimitive();
}  