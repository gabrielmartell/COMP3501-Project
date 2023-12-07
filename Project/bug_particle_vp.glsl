#version 400

// Vertex buffer
in vec3 vertex;
in vec3 normal;
in vec3 color;

// Uniform (global) buffer
uniform mat4 world_mat;
uniform mat4 view_mat;
uniform mat4 normal_mat;
uniform float timer;

// Attributes forwarded to the geometry shader
out vec3 vertex_color;
out float timestep;

// Simulation parameters (constants)
uniform vec3 up_vec = vec3(0.0, 1.0, 0.0);
uniform vec3 object_color = vec3(1.0, 1.0, 0.0);

float speed = 25.0; //!/ Controls the speed of the bees

void main()
{
    //!/ Let time cycle every 4.0208 seconds, which allows the simulation to cycle 16 times in total
    float circtime = timer - 6.0 * floor(timer / 6.0);
    float t = circtime; // Our time parameter
    
    // Let's first work in model space (apply only world matrix)
    vec4 position = world_mat * vec4(vertex, 1.0);

    //!/ Create a new normal that changes each axis, allowing for full motion based on the normal value given, working with time
    vec3 newNormal = normalize(vec3(sin(speed*normal.z + t) * cos(speed*normal.x + t), cos(speed*normal.z + t), sin(speed*normal.z + t) * sin(speed*normal.x + t)));

    vec4 norm = normal_mat * vec4(newNormal, 0.0);

    //!/ Use the norm to add movement to the particles, there is no gravity in this simulation
    position.x += norm.x;
    position.y += norm.y;
    position.z += norm.z;
    
    // Now apply view transformation
    gl_Position = view_mat * position;
        
    // Define outputs
    // Define color of vertex
    //vertex_color = color.rgb; // Color defined during the construction of the particles
    vertex_color = object_color; // Uniform color 

    // Forward time step to geometry shader
    timestep = t;
}
