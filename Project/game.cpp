#define _USE_MATH_DEFINES
#include <iostream>
#include <time.h>
#include <sstream>
#include <cmath>

#include "game.h"
#include "path_config.h"

namespace game {

    // Some configuration constants
    // They are written here as global variables, but ideally they should be loaded from a configuration file

    // Main window settings
    const std::string window_title_g = "A6";
    const unsigned int window_width_g = 800;
    const unsigned int window_height_g = 600;
    const bool window_full_screen_g = false;

    // Viewport and camera settings
    float camera_near_clip_distance_g = 0.01;
    float camera_far_clip_distance_g = 1000.0;
    float camera_fov_g = 20.0; // Field-of-view of camera
    const glm::vec3 viewport_background_color_g(0.0, 0.0, 0.0);
    glm::vec3 camera_position_g(0.5, 1.0, 9.0);
    glm::vec3 camera_look_at_g(9.0, 1.0, 0.5);
    glm::vec3 camera_up_g(0.0, 1.0, 0.0);

    // Materials 
    const std::string material_directory_g = MATERIAL_DIRECTORY;


    Game::Game(void) {

        // Don't do work in the constructor, leave it for the Init() function
    }


    void Game::Init(void) {

        // Run all initialization steps
        InitWindow();
        InitView();
        InitEventHandlers();

        // Set variables
        animating_ = true;
    }


    void Game::InitWindow(void) {

        // Initialize the window management library (GLFW)
        if (!glfwInit()) {
            throw(GameException(std::string("Could not initialize the GLFW library")));
        }

        // Create a window and its OpenGL context
        if (window_full_screen_g) {
            window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), glfwGetPrimaryMonitor(), NULL);
        }
        else {
            window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), NULL, NULL);
        }
        if (!window_) {
            glfwTerminate();
            throw(GameException(std::string("Could not create window")));
        }

        // Make the window's context the current one
        glfwMakeContextCurrent(window_);

        // Initialize the GLEW library to access OpenGL extensions
        // Need to do it after initializing an OpenGL context
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            throw(GameException(std::string("Could not initialize the GLEW library: ") + std::string((const char*)glewGetErrorString(err))));
        }
    }


    void Game::InitView(void) {

        // Set up z-buffer
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // Set viewport
        int width, height;
        glfwGetFramebufferSize(window_, &width, &height);
        glViewport(0, 0, width, height);

        // Set up camera
        // Set current view
        camera_.SetView(camera_position_g, camera_look_at_g, camera_up_g);
        // Set projection
        camera_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);
    }


    void Game::InitEventHandlers(void) {

        // Set event callbacks
        glfwSetKeyCallback(window_, KeyCallback);
        glfwSetFramebufferSizeCallback(window_, ResizeCallback);

        // Set pointer to game object, so that callbacks can access it
        glfwSetWindowUserPointer(window_, (void*)this);
    }


    void Game::SetupResources(void) {

        //!/ Create the heightMap
        heightMap = CreateHeightMap(100, -2, 3, glm::vec2(4,4));

        //!/ Create geometry of the "Plane"
        //! This function uses these parameters, Object Name, Height Map, Grid Width, Grid Length, Number of Quads
        resman_.CreatePlaneWithCraters("CraterPlaneMesh", heightMap, 10, 10, 100);

        // Create geometry of the "wall"
        resman_.CreateTorus("TorusMesh");

        // Load material to be used for normal mapping
        std::string filename = std::string(MATERIAL_DIRECTORY) + std::string("/normal_map");
        resman_.LoadResource(Material, "NormalMapMaterial", filename.c_str());

        // Load texture to be used in normal mapping
        filename = std::string(MATERIAL_DIRECTORY) + std::string("/normal_map2.png");
        resman_.LoadResource(Texture, "NormalMap", filename.c_str());
    }


    void Game::SetupScene(void) {

        // Set background color for the scene
        scene_.SetBackgroundColor(viewport_background_color_g);

        // Create an instance of the wall
        game::SceneNode* wall = CreateInstance("CratePlaneInstance1", "CraterPlaneMesh", "NormalMapMaterial", "NormalMap");
    }


    void Game::MainLoop(void) {

        // Loop while the user did not close the window
        while (!glfwWindowShouldClose(window_)) {
            // Animate the scene
            if (animating_) {
                static double last_time = 0;
                double current_time = glfwGetTime();
                if ((current_time - last_time) > 0.01) {
                    //scene_.Update();

                    // Animate the wall
                    SceneNode* node = scene_.GetNode("CratePlaneInstance1");
                    last_time = current_time;
                }
            }

            // Snap player to the heightmap
            // interpolated height value

            // Draw the scene
            scene_.Draw(&camera_);

            // Push buffer drawn in the background onto the display
            glfwSwapBuffers(window_);

            // Update other events like input handling
            glfwPollEvents();
        }
    }


    void Game::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

        // Get user data with a pointer to the game class
        void* ptr = glfwGetWindowUserPointer(window);
        Game* game = (Game*)ptr;

        // Quit game if 'q' is pressed
        if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        // Stop animation if space bar is pressed
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
            game->animating_ = (game->animating_ == true) ? false : true;
        }

        // View control
        float rot_factor(glm::pi<float>() / 180);
        float trans_factor = 0.1;
        float yDiff;

        //!/ If statement to watch where the camera is when it is moving
        if (key == GLFW_KEY_W || key == GLFW_KEY_S || key == GLFW_KEY_A || key == GLFW_KEY_D) {
             
            //!/ Calculate the y difference of a position, figuring out if the camera should be lower.
            float distanceToCraterCenter = (float) sqrt(pow((game->camera_.GetPosition().x - 4), 2) + pow((game->camera_.GetPosition().z - 4), 2));
            yDiff = game->calculateY(-2, 3, distanceToCraterCenter);
            //printf("%f", yDiff);
            //!/ If the camera is inside the crater radius, is should use the yDiff
            if (distanceToCraterCenter <= 3.0f) {
               game->camera_.SetPosition(glm::vec3(game->camera_.GetPosition().x, 1.0 + yDiff, game->camera_.GetPosition().z));
            }
        }
        if (key == GLFW_KEY_W) {
            printf("%f", game->camera_.GetPosition().y - 1.0);
            if(game->camera_.GetPosition().y - 1.0 >= -1.5){
                game->camera_.Translate(game->camera_.GetForward() * trans_factor);
            }
        }
        if (key == GLFW_KEY_S) {
           if(game->camera_.GetPosition().y - 1.0 >= -1.5){
                game->camera_.Translate(-game->camera_.GetForward() * trans_factor);
            }
        }
        if (key == GLFW_KEY_A) {
            if(game->camera_.GetPosition().y - 1.0 >= -1.5){
                game->camera_.Translate(-game->camera_.GetSide() * trans_factor);
            }
        }
        if (key == GLFW_KEY_D) {
            if(game->camera_.GetPosition().y - 1.0 >= -1.5){
                game->camera_.Translate(game->camera_.GetSide() * trans_factor);
            }
        }
        if (key == GLFW_KEY_L) {
            game->camera_.Yaw(-0.1f);
        }
        if (key == GLFW_KEY_J) {
            game->camera_.Yaw(0.1f);
        }
    }


    void Game::ResizeCallback(GLFWwindow* window, int width, int height) {

        // Set up viewport and camera projection based on new window size
        glViewport(0, 0, width, height);
        void* ptr = glfwGetWindowUserPointer(window);
        Game* game = (Game*)ptr;
        game->camera_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);
    }


    Game::~Game() {

        glfwTerminate();
    }


    Asteroid* Game::CreateAsteroidInstance(std::string entity_name, std::string object_name, std::string material_name) {

        // Get resources
        Resource* geom = resman_.GetResource(object_name);
        if (!geom) {
            throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
        }

        Resource* mat = resman_.GetResource(material_name);
        if (!mat) {
            throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
        }

        // Create asteroid instance
        Asteroid* ast = new Asteroid(entity_name, geom, mat);
        scene_.AddNode(ast);
        return ast;
    }


    void Game::CreateAsteroidField(int num_asteroids) {

        // Create a number of asteroid instances
        for (int i = 0; i < num_asteroids; i++) {
            // Create instance name
            std::stringstream ss;
            ss << i;
            std::string index = ss.str();
            std::string name = "AsteroidInstance" + index;

            // Create asteroid instance
            Asteroid* ast = CreateAsteroidInstance(name, "SimpleSphereMesh", "ObjectMaterial");

            // Set attributes of asteroid: random position, orientation, and
            // angular momentum
            ast->SetPosition(glm::vec3(-300.0 + 600.0 * ((float)rand() / RAND_MAX), -300.0 + 600.0 * ((float)rand() / RAND_MAX), 600.0 * ((float)rand() / RAND_MAX)));
            ast->SetOrientation(glm::normalize(glm::angleAxis(glm::pi<float>() * ((float)rand() / RAND_MAX), glm::vec3(((float)rand() / RAND_MAX), ((float)rand() / RAND_MAX), ((float)rand() / RAND_MAX)))));
            ast->SetAngM(glm::normalize(glm::angleAxis(0.05f * glm::pi<float>() * ((float)rand() / RAND_MAX), glm::vec3(((float)rand() / RAND_MAX), ((float)rand() / RAND_MAX), ((float)rand() / RAND_MAX)))));
        }
    }


    SceneNode* Game::CreateInstance(std::string entity_name, std::string object_name, std::string material_name, std::string texture_name) {

        Resource* geom = resman_.GetResource(object_name);
        if (!geom) {
            throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
        }

        Resource* mat = resman_.GetResource(material_name);
        if (!mat) {
            throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
        }

        Resource* tex = NULL;
        if (texture_name != "") {
            tex = resman_.GetResource(texture_name);
            if (!tex) {
                throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
            }
        }

        SceneNode* scn = scene_.CreateNode(entity_name, geom, mat, tex);
        return scn;
    }

    //!/ Create the height map
    //! This function uses these parameters, Number of Quads, Crater Depth, Crater Rad, Crater Position
    //! This function could be easily changed to include number of craters to allow for more craters to be added.
    GLfloat* Game::CreateHeightMap(float numQuads, float craterDep, float craterRad, glm::vec2 craterPos) {

        //!/ Quad Settings and variables
        int v_gWidth = sqrt(numQuads) + 1;
        int v_gLength = sqrt(numQuads) + 1;

        //!/ Height Array
        GLfloat* vertexHeight = new GLfloat[v_gWidth * v_gLength];

        printf("HEIGHT MAP:\n");
        for (int heightX = 0; heightX < v_gWidth; heightX++) {
            for (int heightZ = 0; heightZ < v_gLength; heightZ++) {

                //!/ Calculate the distance to the center of the crater
                float distanceToCenter = (float)sqrt(pow((heightX - craterPos.x), 2) + pow((heightZ - craterPos.y), 2));

                //!/ Calculate the y value based off of a cosin wave, 1/2a * cos(pi*x/b) + 1/2c, where c = a, a = craterDepth, b = distanceToCenter
                float receivedYValue = calculateY(craterDep, craterRad, distanceToCenter);

                //!/ If the distance is closer to to the center
                if (distanceToCenter <= craterRad) {
                    //!/ If the height value received is smaller than 0, it is correct
                    if ((receivedYValue) < 0) {
                        vertexHeight[heightZ + (heightX * v_gLength)] = receivedYValue;
                        printf("%.2f  ", receivedYValue);
                    }
                    else {
                        //!/ If the value is larger than 0, it should be 0
                        vertexHeight[heightZ + (heightX * v_gLength)] = 0;
                        printf("%.2f  ", 0.0f);
                    }
                }
                else {
                    //!/ If not, the value should be 0 and the vertex flat
                    vertexHeight[heightZ + (heightX * v_gLength)] = 0;
                    printf("%.2f  ", 0.0f);
                }
            }
            printf("\n");
        }

        return vertexHeight;
    }

    //!/ Calculate the Y value
    //! This function calculates the current y value based on the distance
    float Game::calculateY(float dep, float rad, float dis){
        return (0.5 * dep) * cos((M_PI * dis) / rad) + (0.5 * dep);
    }

} // namespace game