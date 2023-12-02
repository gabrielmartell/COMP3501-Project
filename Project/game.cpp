#define _USE_MATH_DEFINES
#include <iostream>
#include <time.h>
#include <sstream>
#include <cmath>
#include <random>

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
    float camera_fov_g = 90.0; // Field-of-view of camera
    const glm::vec3 viewport_background_color_g(0.0, 0.0, 0.0);
    glm::vec3 camera_position_g(30.0, 1.0, 9.0);
    glm::vec3 camera_look_at_g(9.0, 1.0, 0.5);
    glm::vec3 camera_up_g(0.0, 1.0, 0.0);

    //Switching to Arrow Key Movement
    bool upPressed = false;
    bool downPressed = false;
    bool leftPressed = false;
    bool rightPressed = false;
    bool usingMouseCamera = true;

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
        heightMap = CreateHeightMap(50, 50, 3.0);

        //!/ Create geometry of the "Plane"
        //! This function uses these parameters, Object Name, Height Map, Grid Width, Grid Length, Number of Quads
        resman_.CreatePlaneWithCraters("GameMapMesh", heightMap, 50, 50, 50, 50);

        // Create geometry of the "wall"
        resman_.CreateTorus("TorusMesh");


        /*
        ======================
        MATERIALS
        ======================
        */
        std::string filename = std::string(MATERIAL_DIRECTORY) + std::string("/normal_map");
        resman_.LoadResource(Material, "NormalMapMaterial", filename.c_str());

        filename = std::string(MATERIAL_DIRECTORY) + std::string("/textured_material");
        resman_.LoadResource(Material, "TexturedMaterial", filename.c_str());

        /*
        ======================
        MESHES
        ======================
        */
        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/mushroom.obj");
        resman_.LoadResource(Mesh, "Mushroom", filename.c_str());

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/treebottom.obj");
        resman_.LoadResource(Mesh, "TreeTrunk", filename.c_str());

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/treetop.obj");
        resman_.LoadResource(Mesh, "TreeTop", filename.c_str());

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/bush.obj");
        resman_.LoadResource(Mesh, "Bush", filename.c_str());

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/wall_door.obj");
        resman_.LoadResource(Mesh, "WallDoor", filename.c_str());

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/wall_full.obj");
        resman_.LoadResource(Mesh, "WallFull", filename.c_str());

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/wall_roof.obj");
        resman_.LoadResource(Mesh, "WallRoof", filename.c_str());

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/wall_window.obj");
        resman_.LoadResource(Mesh, "WallWindow", filename.c_str());

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/roof_main.obj");
        resman_.LoadResource(Mesh, "RoofMain", filename.c_str());

        /*
        ======================
        TEXTURES
        ======================
        */
        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/normal_map2.png");
        resman_.LoadResource(Texture, "NormalMap", filename.c_str());

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/mushroom_text.png");
        resman_.LoadResource(Texture, "MushroomTexture", filename.c_str());

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/bark.png");
        resman_.LoadResource(Texture, "TreeBark", filename.c_str());

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/leaves.png");
        resman_.LoadResource(Texture, "TreeLeaves", filename.c_str());
    }


    void Game::SetupScene(void) {

        // Set background color for the scene
        scene_.SetBackgroundColor(viewport_background_color_g);

        glm::vec3 cabin_location(10.0, 3.0, 25.0);
        CreateCabin(cabin_location);

        CreateProps(50, 30, cabin_location);

        // Create an instance of the map
        game::SceneNode* map = CreateInstance("MapInstance1", "GameMapMesh", "TexturedMaterial", "TreeLeaves");
    }



    void Game::MainLoop(void) {

        float previousVer = 0.0f;
        float horizontalAngle = 0.0f;
        float verticalAngle = 0.0f;

        float mouseSpeed = 0.01f;

        double xpos, ypos;

        //game::SceneNode* treetrunk1 = CreateInstance("TreeTrunk", "TreeTrunk", "TexturedMaterial", "MushroomTexture");

       // game::SceneNode* mushroom = CreateInstance("Mushroom", "Mushroom", "TexturedMaterial", "MushroomTexture");
        //mushroom->SetScale(glm::vec3(0.1, 0.1, 0.1));

        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);



        // Loop while the user did not close the window
        while (!glfwWindowShouldClose(window_)) {
            glfwGetCursorPos(window_, &xpos, &ypos);
            glfwSetCursorPos(window_, window_width_g / 2, window_height_g / 2);

            static double last_time = 0;
            double current_time = glfwGetTime();

            horizontalAngle += mouseSpeed * (current_time - last_time) * float(window_width_g / 2 - xpos);
            verticalAngle += mouseSpeed * (current_time - last_time) * float(window_height_g / 2 - ypos);

            //printf("x = %.02f, y = %.02f\n", xpos, ypos);

            // Animate the scene
            if (animating_) {
                static double last_time = 0;
                double current_time = glfwGetTime();
                if ((current_time - last_time) > 0.01) {
                    /*
                    =========================================================
                    MAIN LOOP
                    =========================================================
                    */

                    // Camera Movement and Handle
                    camera_.Yaw(glm::radians(horizontalAngle));
                    if (camera_.GetUp().y > 0.1f) {
                        camera_.Pitch(glm::radians(verticalAngle));
                        if (verticalAngle != 0.0f) {
                            previousVer = verticalAngle;
                        }
                    }
                    else {
                        if (previousVer < 0 && verticalAngle > 0) {
                            camera_.Pitch(glm::radians(verticalAngle));
                        }
                        if (previousVer > 0 && verticalAngle < 0) {
                            camera_.Pitch(glm::radians(verticalAngle));
                        }
                    }
                    camera_.Roll(glm::radians(0.0f));

                    CollisionDetection();
                    //scene_.Update();

                    // Animate the wall
                    SceneNode* node = scene_.GetNode("MapInstance1");
                    //SceneNode* node = scene_.GetNode("CratePlaneInstance1");
                    //glm::quat rotation = glm::angleAxis(glm::pi<float>() / 180.0f, glm::vec3(0.0, 1.0, 0.0));
                    //node->Rotate(rotation);
                    last_time = current_time;
                }
            }

            printf("x = %f, z = %f\n", camera_.GetPosition().x, camera_.GetPosition().z);
            CollisionDetection();

            // Draw the scene
            scene_.Draw(&camera_);

            // Push buffer drawn in the background onto the display
            glfwSwapBuffers(window_);

            // Update other events like input handling
            glfwPollEvents();

            horizontalAngle = 0.0f;
            // vertical angle : 0, look at the horizon
            verticalAngle = 0.0f;
        }
    }

    void Game::CollisionDetection() {

        // Determine current player position
        glm::vec3 playerPosition = camera_.GetPosition();

        for (auto it = scene_.begin(); it != scene_.end(); ++it) {
            SceneNode* currentObj = *it;

            // Collision for Trees
            if ((currentObj->GetName()).find("TreeTrunk") != std::string::npos) {

            }
            // Collision for ALL Cabin Walls (Not the Entrance) - THESE SHOULD BE ABSOLUTELY SOLID THE PLAYER CANNOT GO THROUGH THE WINDOW BECAUSE THEY SUCK
            if ((currentObj->GetName()).find("Wall") != std::string::npos) {

            }
            // Collision for ALL ENTRANCES (Not the Walls) - There are two entrances, same orientation, so collision for detecting the door should be the same.
            if ((currentObj->GetName()).find("CabinEntrance") != std::string::npos) {

            }

            // Collision for Mushroom
            if ((currentObj->GetName()).find("Mushroom") != std::string::npos) {

            }
            // Collision for Bushes
            if ((currentObj->GetName()).find("Bush") != std::string::npos) {

            }
            // Collision for Bees
            if ((currentObj->GetName()).find("Bees") != std::string::npos) {

            }

            // Collision for Nail
            if ((currentObj->GetName()).find("Nail") != std::string::npos) {

            }

        }
    }

    void Game::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

        // Get user data with a pointer to the game class
        void* ptr = glfwGetWindowUserPointer(window);
        Game* game = (Game*)ptr;
        double lastToggleTime = 0.0;
        const double toggleDelay = 0.5;

        // Quit game if 'q' is pressed
        if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        // Stop animation if space bar is pressed
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
            game->animating_ = (game->animating_ == true) ? false : true;
        }

        //!/ View control
        float rot_factor(glm::pi<float>() / 180);
        float trans_factor = 0.1;

        //!/ WASD movment
        if (key == GLFW_KEY_W) {
            printf("%f", game->camera_.GetPosition().y - 1.0);
            if (game->camera_.GetPosition().y - 1.0 >= -1.5) {

                game->camera_.Translate(game->camera_.GetForward() * trans_factor);
            }
        }
        if (key == GLFW_KEY_S) {
            if (game->camera_.GetPosition().y - 1.0 >= -1.5) {
                game->camera_.Translate(-game->camera_.GetForward() * trans_factor);
            }
        }
        if (key == GLFW_KEY_A) {
            if (game->camera_.GetPosition().y - 1.0 >= -1.5) {
                game->camera_.Translate(-game->camera_.GetSide() * trans_factor);
            }
        }
        if (key == GLFW_KEY_D) {
            if (game->camera_.GetPosition().y - 1.0 >= -1.5) {
                game->camera_.Translate(game->camera_.GetSide() * trans_factor);
            }
        }

        //!/ Camera control
        if (key == GLFW_KEY_UP && !usingMouseCamera) {
			game->camera_.Pitch(rot_factor);
        }
        if (key == GLFW_KEY_DOWN && !usingMouseCamera) {
			game->camera_.Pitch(-rot_factor);
        }
        if (key == GLFW_KEY_LEFT && !usingMouseCamera) {
			game->camera_.Yaw(rot_factor);
        }
        if (key == GLFW_KEY_RIGHT && !usingMouseCamera) {
			game->camera_.Yaw(-rot_factor);
        }

        //!/ Caps lock button to change mode of camera: Mouse vs Tank
        if (key == GLFW_KEY_CAPS_LOCK && action == GLFW_PRESS) {
            double currentTime = glfwGetTime(); 
            if (currentTime - lastToggleTime > toggleDelay) {
                usingMouseCamera = !usingMouseCamera;
                printf("usingMouseCamera: %d\n", usingMouseCamera);
                lastToggleTime = currentTime; 
            }
        }
        
        //!/ Space button to create upwards movment
        if (key == GLFW_KEY_SPACE) {
            if (game->camera_.GetPosition().y - 1.0 >= -1.5) {
                game->camera_.Translate(game->camera_.GetUp() * trans_factor);
            }
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

    void Game::CreateCabin(glm::vec3 location) {

        float location_x = location.x;
        float location_y = location.y;
        float location_z = location.z;

        // A wall is of ~6.6 length, and ~1.7 height. - Gabe

        game::SceneNode* wallEntrance = CreateInstance("CabinEntrance", "WallDoor", "TexturedMaterial", "TreeBark");
        game::SceneNode* wallEntrance2 = CreateInstance("CabinEntrance2", "WallDoor", "TexturedMaterial", "TreeBark");

        game::SceneNode* wallRoof = CreateInstance("WallRoof", "WallRoof", "TexturedMaterial", "TreeBark");
        game::SceneNode* wallRoof2 = CreateInstance("WallRoof2", "WallRoof", "TexturedMaterial", "TreeBark");

        game::SceneNode* roofMain = CreateInstance("Roof", "RoofMain", "TexturedMaterial", "TreeBark");

        game::SceneNode* wallWindow = CreateInstance("WallWindow", "WallWindow", "TexturedMaterial");
        game::SceneNode* wallFull = CreateInstance("WallFull", "WallFull", "TexturedMaterial");
        game::SceneNode* floor = CreateInstance("Floor", "WallFull", "TexturedMaterial");


        wallEntrance->SetPosition(glm::vec3(location_x, location_y, location_z));
        wallEntrance->SetScale(glm::vec3(0.5, 0.5, 0.5));

        wallEntrance2->SetPosition(glm::vec3(location_x, location_y, location_z+6.6f));
        wallEntrance2->SetScale(glm::vec3(0.5, 0.5, 0.5));

        wallRoof->SetPosition(glm::vec3(location_x, location_y+1.7, location_z));
        wallRoof->SetScale(glm::vec3(0.5, 0.5, 0.5));

        wallRoof2->SetPosition(glm::vec3(location_x, location_y+1.7, location_z + 6.6f));
        wallRoof2->SetScale(glm::vec3(0.5, 0.5, 0.5));

        roofMain->SetPosition(glm::vec3(location_x, location_y + 1.7, location_z + 3.3f));
        roofMain->SetScale(glm::vec3(0.4, 0.4, 3.3));

        wallWindow->SetPosition(glm::vec3(location_x+3.3f, location_y, location_z+3.3f));
        wallWindow->SetScale(glm::vec3(0.5, 0.5, 0.5));
        wallWindow->Rotate(glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0)));

        wallFull->SetPosition(glm::vec3(location_x-3.3f, location_y, location_z+3.3f));
        wallFull->SetScale(glm::vec3(0.5, 0.5, 0.5));
        wallFull->Rotate(glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0)));

        floor->SetPosition(glm::vec3(location_x, location_y, location_z));
        floor->SetScale(glm::vec3(0.5, 1.1, 0.5));
        floor->Rotate(glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0)));

    }

    void Game::CreateProps(int treeNum, int bushNum, glm::vec3 cabin_location) {

        // TREE CREATION
        for (int i = 0; i < treeNum; ++i) {
            std::stringstream ss;
            ss << i;
            std::string index = ss.str();
            std::string name = "TreeTrunk" + index;
            std::string name2 = "TreeTop" + index;

            game::SceneNode* treeTrunk = CreateInstance(name, "TreeTrunk", "TexturedMaterial", "TreeBark");
            game::SceneNode* treeTop = CreateInstance(name2, "TreeTop", "TexturedMaterial", "TreeLeaves");

            bool locationFound = false;

            while (!locationFound) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> x_placementDist(0, 50);
                std::uniform_int_distribution<int> z_placementDist(0, 50);

                std::uniform_int_distribution<int> heightDist(-1, 0);
                std::uniform_int_distribution<int> angleDist(-10, 10);

                int random_x = x_placementDist(gen);
                int random_z = z_placementDist(gen);
                int random_y = heightDist(gen) + heightMap[random_z + (random_x * 50)];
                int random_ang = angleDist(gen);

                if (!((random_x < cabin_location.x + 5 && random_x > cabin_location.x - 15) && (random_z < cabin_location.z + 5 && random_x > cabin_location.z - 15))) {
                    locationFound = true;
                    treeTrunk->SetPosition(glm::vec3(random_x, random_y, random_z));
                    treeTrunk->Rotate(glm::angleAxis((float)random_ang, glm::vec3(0.0, 1.0, 0.0)));

                    treeTop->SetPosition(glm::vec3(random_x, random_y, random_z));
                    treeTop->Rotate(glm::angleAxis(glm::radians((float)random_ang), glm::vec3(0.0, 1.0, 0.0)));
                }
            }
        }

        // BUSH CREATION
        for (int i = 0; i < bushNum; ++i) {
            std::stringstream ss;
            ss << i;
            std::string index = ss.str();
            std::string name = "Bush" + index;
            game::SceneNode* bush = CreateInstance(name, "Bush", "TexturedMaterial", "TreeLeaves");

            bool locationFound = false;
            while (!locationFound) {

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> placementDist(0, 50);
                std::uniform_int_distribution<int> angleDist(0, 50);

                int random_x = placementDist(gen);
                int random_z = placementDist(gen);

                int random_ang = angleDist(gen);
                if (!((random_x < cabin_location.x + 5 && random_x > cabin_location.x - 15) && (random_z < cabin_location.z + 5 && random_x > cabin_location.z - 15))) {
                    locationFound = true;
                    bush->SetPosition(glm::vec3(random_x, heightMap[random_z + (random_x * 50)], random_z));
                    bush->SetScale(glm::vec3(0.7, 0.7, 0.7));
                    bush->Rotate(glm::angleAxis((float)random_ang, glm::vec3(0.0, 1.0, 0.0)));
                }
            }
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
    GLfloat* Game::CreateHeightMap(int v_gWidth, int v_gLength, float hillHeight) {

        //!/ Height Array
        GLfloat* vertexHeight = new GLfloat[v_gWidth * v_gLength];

        printf("HEIGHT MAP:\n");
        for (int heightX = 0; heightX < v_gWidth; heightX++) {
            for (int heightZ = 0; heightZ < v_gLength; heightZ++) {

                //!/ If statement to divide the map into 3 sections, hill, slope and field
                if (heightX <= (v_gWidth / 3)) {
					vertexHeight[heightZ + (heightX * v_gLength)] = 3.0;
					printf("%.2f  ", hillHeight);
				}
				else if (heightX <= (v_gWidth * 2 / 3)) {
					if (heightZ <= (v_gLength * 1 / 2)) {
						float newY = hillHeight * cos((M_PI / fabs(v_gWidth * 2 / 3)) * heightX);

						vertexHeight[heightZ + (heightX * v_gLength)] = hillHeight + newY;
						printf("%.2f  ", hillHeight + newY);
					}
					else {
						vertexHeight[heightZ + (heightX * v_gLength)] = 0.0;
						printf("%.2f  ", 0.0f);
					}
				}
				else {
					vertexHeight[heightZ + (heightX * v_gLength)] = 0.0;
					printf("%.2f  ", 0.0f);
				}

            }
            printf("\n");
        }

        return vertexHeight;
    }

} // namespace game
