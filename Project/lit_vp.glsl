#version 130

// Vertex buffer
in vec3 vertex;
in vec3 normal;
in vec2 uv;

// Uniform (global) buffer
uniform mat4 world_mat;
uniform mat4 view_mat;
uniform mat4 projection_mat;
uniform mat4 normal_mat;

// Attributes forwarded to the fragment shader
out vec3 position_interp;
out vec3 normal_interp;
out vec2 uv_interp;
out vec3 light_pos;

// Material attributes (constants)
uniform vec3 light_position; // Light position for the flashlight

void main()
{
    // Transform vertex position into clip space
    gl_Position = projection_mat * view_mat * world_mat * vec4(vertex, 1.0);

    // Transform vertex position and normal into view space
    position_interp = vec3(view_mat * world_mat * vec4(vertex, 1.0));
    normal_interp = normalize(vec3(normal_mat * vec4(normal, 0.0)));

    // Pass through the texture coordinates
    uv_interp = uv;

    // Transform light position into view space
    light_pos = vec3(view_mat * vec4(light_position, 1.0));
}
