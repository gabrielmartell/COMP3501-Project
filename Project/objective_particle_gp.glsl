#version 400

// Definition of the geometry shader
layout (points) in;
layout (triangle_strip, max_vertices = 7) out;

// Attributes passed from the vertex shader
in vec3 vertex_color[];
in float timestep[];

// Uniform (global) buffer
uniform mat4 projection_mat;

// Simulation parameters (constants)
uniform float particle_size = 0.01;

// Attributes passed to the fragment shader
out vec4 frag_color;

// add any other attributes you want here

void main(void){

    // Get the position of the particle
    vec4 position = gl_in[0].gl_Position;

    // Define particle size
    float p_size = particle_size*5;

    // Define the positions of the six vertices that will form a hexagons? I was trying to make hexagons but these shapes are cooler
    vec4 v[6];
    for (int i = 0; i < 6; ++i) {
        float angle = float(i) * 2.0 * 3.14159 / 6.0;
        v[i] = vec4(position.x + p_size * cos(angle), position.y + p_size * sin(angle), position.z, 1.0);
    }
	

    // Create the new geometry: a quad with 6 vertices
    for (int i = 0; i < 6; i++){
	
        gl_Position = projection_mat * v[i];
        frag_color = vec4(vertex_color[0], 1.0);
        EmitVertex();
     }

     EndPrimitive();
}
 