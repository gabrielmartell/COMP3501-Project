#version 130

// Attributes passed from the vertex shader
in vec3 vertex_position;
in vec2 vertex_uv;
in vec3 frag_normal;
in vec3 frag_tangent;
in vec3 frag_bitangent;
in vec3 light_pos;

// Uniform (global) buffer
uniform sampler2D texture_map; // Normal map

// Material attributes (constants)
uniform vec4 object_color = vec4(0.79, 0.96, 0.60, 1.0);

out vec4 frag_color;

void main() 
{
    // Sample the normal map
    vec3 map_normal = texture2D(texture_map, vertex_uv).rgb;
    map_normal = map_normal * 2.0 - 1.0; // Transform from [0,1] to [-1,1]
    map_normal.y = -map_normal.y; // Invert the y component of the normal

    // Construct the TBN matrix from the interpolated tangent, bitangent, and normal
    mat3 TBN = mat3(frag_tangent, frag_bitangent, frag_normal);

    // Transform the normal from the normal map to view space
    vec3 N = normalize(TBN * map_normal);
    
    // Lighting calculations in view space
    vec3 L = normalize(light_pos - vertex_position); // Light direction
    
    // Compute the diffuse component
    float lambertian = max(dot(N, L), 0.0);

    // Ambient component
    float ambient = 0.3; // Ambient light level

    // Combine the ambient and diffuse components and multiply by the object color
    vec3 color = (ambient + lambertian) * object_color.rgb;
    
    // Output the final color
    frag_color = vec4(color, object_color.a);
}