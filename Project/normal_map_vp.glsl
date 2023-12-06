#version 130

// Vertex buffer
in vec3 vertex;
in vec3 normal;
in vec2 uv;
in vec3 tangent; // Added tangent as a proper attribute

// Uniform (global) buffer
uniform mat4 world_mat;
uniform mat4 view_mat;
uniform mat4 projection_mat;
uniform mat4 normal_mat; // Used for transforming normals

// Attributes forwarded to the fragment shader
out vec3 vertex_position;
out vec2 vertex_uv;
out vec3 frag_normal;
out vec3 frag_tangent;
out vec3 frag_bitangent;
out vec3 light_pos;

// Material attributes (constants)
uniform vec3 light_position = vec3(-0.5, -0.5, 1.5);

void main()
{
    gl_Position = projection_mat * view_mat * world_mat * vec4(vertex, 1.0);

    // Transform vertex position & normal into view space
    vertex_position = vec3(view_mat * world_mat * vec4(vertex, 1.0));
    frag_normal = normalize(vec3(normal_mat * vec4(normal, 0.0)));
    
    // Transform the tangent vector and compute the bitangent
    frag_tangent = normalize(vec3(normal_mat * vec4(tangent, 0.0)));
    frag_bitangent = cross(frag_normal, frag_tangent);
    
    // Transform light position into view space
    light_pos = vec3(view_mat * vec4(light_position, 1.0));

    // Pass through the texture coordinates
    vertex_uv = uv; 
}