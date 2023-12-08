#version 400

// Vertex buffer
in vec3 vertex;
in vec3 normal;
in vec3 color;
in vec2 uv;

// Uniform (global) buffer
uniform mat4 world_mat;
uniform mat4 view_mat;
uniform mat4 normal_mat;
uniform float timer;

// Attributes forwarded to the geometry shader
out vec3 vertex_color;
out vec2 vertex_uv;
out float timestep;

// Simulation parameters (constants)
uniform float particle_count = 500.0;
uniform float particle_radius = 1.0;
uniform float particle_jump_height = 0.5;

void main()
{
    //!/ Let time cycle every 6.3 seconds
    float circtime = timer - 6.3 * floor(timer / 6.3);
    float t = circtime; // Our time parameter

    // Let's first work in model space (apply only world matrix)
    vec4 position = world_mat * vec4(vertex, 1.0);
    
    //!/ Make particles jump up and fall down
    float jump = particle_jump_height * sin(t * 3); // Use sine function for smoother motion

    //!/ Distribute particles across the entire circular region
    float angle = mod(float(gl_VertexID), particle_count) / particle_count * 2.0 * 3.14159;

    //!/ Create a normal to direct our particles
    vec3 newNormal = vec3(particle_radius * cos(angle * t), jump + 0.5,particle_radius * sin(angle * t));

    vec4 norm = normal_mat * vec4(newNormal, 0.0);

    //!/ Move the x, y and z around;
    position.x += norm.x;
    position.y += norm.y;
    position.z += norm.z;


    // Now apply view transformation
    gl_Position = view_mat * position;
    vertex_uv = uv;

    // Define outputs
    vertex_color = vec3(0.5, 1.0, 0.5);
    timestep = t;
}