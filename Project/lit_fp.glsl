#version 130

// Attributes passed from the vertex shader
in vec3 position_interp;
in vec2 uv_interp;
in vec3 light_pos; // Position of the light source

// Uniform (global) buffer
uniform sampler2D texture_map;
uniform vec3 camera_pos; // Position of the camera
uniform float max_distance; // Maximum distance the light reaches

void main() 
{
    // Retrieve texture value
    vec4 textureColor = texture(texture_map, uv_interp);

    // Calculate distance from the light source to the fragment
    float distance = length(light_pos - position_interp);

    // Attenuate based on distance
    float attenuation = clamp(1.0 - (distance / max_distance), 0.0, 1.0);

    // Apply the attenuation to the texture color
    vec3 illuminatedColor = attenuation * textureColor.rgb;

    // Output final color
    gl_FragColor = vec4(illuminatedColor, textureColor.a);
}