#version 130

// Passed from the vertex shader
in vec2 uv0;

// Passed from outside
uniform float timer;
uniform sampler2D texture_map;

void main() 
{
    vec2 pos = uv0;
    float wave = 0.05 * sin(timer * 5.0 + uv0.y * 10.0); // Wavy effect
    pos.x += wave;

    vec4 originalColor = texture(texture_map, pos);
    vec4 redTint = vec4(1.0, 0.5, 0.5, 1.0); // Adjust the red tint intensity

    // Blend the original color with the red tint
    vec4 blendedColor = mix(originalColor, redTint, 0.5); // Adjust the blend factor

    gl_FragColor = blendedColor;
}
