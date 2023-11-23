#version 420 core
layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vec3 te_position[];
in vec2 uv[];

out vec2 uv_coords;
out vec3 frag_normal;
out vec3 frag_position;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;

    float near;
    float far;
    float window_width;
    float window_height;
};
uniform mat4 model;

void main() {
	mat4 view_model = view * model;
	mat3 normal_matrix = mat3(view_model);

	vec3 a = te_position[2] - te_position[0];
	vec3 b = te_position[1] - te_position[0];
	vec3 normal = normalize(cross(a, b));
	frag_normal = normal;

	for (int i = 0; i < gl_in.length(); i++) {
		uv_coords = uv[i];

		frag_position = te_position[i];
		gl_Position = projection * view * vec4(frag_position, 1);
		EmitVertex();
	}

	EndPrimitive();
}