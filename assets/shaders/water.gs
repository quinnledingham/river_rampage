#version 420 core
layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vec2 uv[];
in vec4 normal[];

out vec3 frag_normal;
out vec3 frag_tangent;
out vec3 frag_bitangent;
out vec3 frag_position;
out vec2 uv_coords;

void main(void)
{
	vec3 b = (gl_in[1].gl_Position - gl_in[0].gl_Position).xyz;
    vec3 a = (gl_in[2].gl_Position - gl_in[0].gl_Position).xyz;
    vec3 N = normalize(cross(b, a));

    for(int i = 0; i < gl_in.length(); i++)
    {
        frag_normal = normal[i].xyz;
        //frag_normal = -N;

		uv_coords = uv[i];
		vec4 pos = gl_in[i].gl_Position;
		frag_position = pos.xyz;

        gl_Position = pos;
        EmitVertex();
    }

    EndPrimitive();
}  