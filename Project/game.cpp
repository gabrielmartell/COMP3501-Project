#define _USE_MATH_DEFINES
#include <iostream>
#include <time.h>
#include <sstream>
#include <cmath>
#include <random>

#include "game.h"
#include "path_config.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace game {

    // Some configuration constants
    // They are written here as global variables, but ideally they should be loaded from a configuration file

    // Main window settings
    const std::string window_title_g = "Hungry Man";
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

    //!/ Switching to Arrow Key Movement
    bool upPressed = false;
    bool downPressed = false;
    bool leftPressed = false;
    bool rightPressed = false;
    bool usingMouseCamera = true;
    
    //!/ Crouching Boolean
    bool isCrouching = false;
    bool isHidden = false;

    //!/ This will control whether UI from IMGUI is on
    bool usingUI = true;
    bool isDead = false;
    bool game_is_over = false;

    float hungry_speed = 0.2;

    //!/ Random Variables
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist(0.0f, 360.0f);

    //!/ Global map width and height variables
    int v_gWidthReal = 50.0;
    int v_gLengthReal = 50.0;

    //!/ Collision detection
    // We need a way to move the player back to their last position
    glm::vec3 lastPosition;
    bool inCabin = false;

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

        //!/ Initilization of Collectibles

        gameScore = glm::vec4(0,0,0,0);

        //ImGui initialization code
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL3_Init();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
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

        /*
        ======================
        MATERIALS
        ======================
        */
        printf("    MATERIALS [");
        std::string filename = std::string(MATERIAL_DIRECTORY) + std::string("/normal_map");
        resman_.LoadResource(Material, "NormalMapMaterial", filename.c_str());
        printf("|");

        // Load material for screen-space effect
        filename = std::string(MATERIAL_DIRECTORY) + std::string("/screen_space");
        resman_.LoadResource(Material, "ScreenSpaceMaterial", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("/textured_material");
        resman_.LoadResource(Material, "TexturedMaterial", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("/bug_particle");
        resman_.LoadResource(Material, "SwarmMaterial", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("/objective_particle");
        resman_.LoadResource(Material, "ObjectiveMaterial", filename.c_str());
        printf("|]\n");

        /*
        ======================
        MESHES
        ======================
        */

        printf("    MESHES [");
        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/mushroom.obj");
        resman_.LoadResource(Mesh, "Mushroom", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/nail.obj");
        resman_.LoadResource(Mesh, "Nail", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/treebottom.obj");
        resman_.LoadResource(Mesh, "TreeTrunk", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/treetop.obj");
        resman_.LoadResource(Mesh, "TreeTop", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/bush.obj");
        resman_.LoadResource(Mesh, "Bush", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/wall_door.obj");
        resman_.LoadResource(Mesh, "WallDoor", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/wall_full.obj");
        resman_.LoadResource(Mesh, "WallFull", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/wall_roof.obj");
        resman_.LoadResource(Mesh, "WallRoof", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/wall_window.obj");
        resman_.LoadResource(Mesh, "WallWindow", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/roof_main.obj");
        resman_.LoadResource(Mesh, "RoofMain", filename.c_str());
        printf("|");

        // HUNGRY-MAN PARTS
        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/hungryhead.obj");
        resman_.LoadResource(Mesh, "HungryHead", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/hungryeyes.obj");
        resman_.LoadResource(Mesh, "HungryEyes", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/hungrytongue.obj");
        resman_.LoadResource(Mesh, "HungryTongue", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/hungrytorso.obj");
        resman_.LoadResource(Mesh, "HungryTorso", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/hungryrightarm.obj");
        resman_.LoadResource(Mesh, "HungryRArm", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/hungryleftarm.obj");
        resman_.LoadResource(Mesh, "HungryLArm", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/hungryrightleg.obj");
        resman_.LoadResource(Mesh, "HungryRLeg", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\models/hungryleftleg.obj");
        resman_.LoadResource(Mesh, "HungryLLeg", filename.c_str());
        printf("|]\n");

        /*
        ======================
        TEXTURES
        ======================
        */
        printf("    TEXTURES [");
        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/normal_map2.png");
        resman_.LoadResource(Texture, "NormalMap", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/mushroom_text.png");
        resman_.LoadResource(Texture, "MushroomTexture", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/skybox.png");
        resman_.LoadResource(Texture, "Skybox", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/rust.png");
        resman_.LoadResource(Texture, "NailTexture", filename.c_str());

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/bark.png");
        resman_.LoadResource(Texture, "TreeBark", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/grass.png");
        resman_.LoadResource(Texture, "GrassTexture", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/leaves.png");
        resman_.LoadResource(Texture, "TreeLeaves", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/orange.png");
        resman_.LoadResource(Texture, "HungrySkin", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/hungryeyes.png");
        resman_.LoadResource(Texture, "HungryEyesText", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/pink.png");
        resman_.LoadResource(Texture, "HungryTongueText", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/yum.png");
        resman_.LoadResource(Texture, "Yum", filename.c_str());
        printf("|");

        filename = std::string(MATERIAL_DIRECTORY) + std::string("\\textures/hungryman.png");
        resman_.LoadResource(Texture, "HungryManPic", filename.c_str());

        printf("|]\n");


        //!/ Create the heightMap
        //!/ The values can be changed at the top since they're global
        //!/ I swear there's a good reason
        printf("    MAP [");
        heightMap = CreateHeightMap(v_gWidthReal, v_gLengthReal, 3.0);

        //!/ Create geometry of the "Plane"
        //! This function uses these parameters, Object Name, Height Map, Grid Width, Grid Length, Number of Quads
        resman_.CreateMapPlane("GameMapMesh", heightMap, 50, 50, 50, 50);
        printf("|");

        // Create geometry of the "wall"
        resman_.CreateBugParticles("BeeParticles", 10);
        resman_.CreateSphereParticles("SphereParticles");
        printf("|]\n");


        // Setup drawing to texture
        scene_.SetupDrawToTexture();
    }

    void Game::SetupScene(void) {

        // Set background color for the scene
        scene_.SetBackgroundColor(viewport_background_color_g);

        //!/ Define the Cabin location and Hungry man's location
        glm::vec3 cabin_location(10.0, 3.0, 25.0);
        glm::vec3 hungry_location(50.0, 0.0, 50.0);

        //!/ Functions to create the cabin, the trees/bushes and Hungry man themself
        CreateCabin(cabin_location);

        CreateProps(50, 30, cabin_location);

        CreateCollectibles(3, 3, 3, cabin_location);

        CreateHungry(hungry_location);


        // Create an instance of the map
        //game::SceneNode* map = CreateInstance("MapInstance1", "GameMapMesh", "TexturedMaterial", "TreeLeaves");
        
      /*
        ====================================================
        Skybox Creation
        ====================================================
      */
        game::SceneNode* skyboxTop = CreateInstance("SkyboxInstance1", "GameMapMesh", "TexturedMaterial", "Skybox");
        game::SceneNode* skyboxFront = CreateInstance("SkyboxInstance2", "GameMapMesh", "TexturedMaterial", "Skybox");
        game::SceneNode* skyboxBack = CreateInstance("SkyboxInstance3", "GameMapMesh", "TexturedMaterial", "Skybox");
        game::SceneNode* skyboxLeft = CreateInstance("SkyboxInstance4", "GameMapMesh", "TexturedMaterial", "Skybox");
        game::SceneNode* skyboxRight = CreateInstance("SkyboxInstance5", "GameMapMesh", "TexturedMaterial", "Skybox");

        int skyboxScale = 15;

        skyboxTop->SetPosition(glm::vec3(-100, 7, -100));
        skyboxTop->SetScale(glm::vec3(skyboxScale, skyboxScale, skyboxScale));

        skyboxFront->SetPosition(glm::vec3(-100, 100, -100));
        skyboxFront->SetScale(glm::vec3(skyboxScale, skyboxScale, skyboxScale));
        skyboxFront->Rotate(glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0)));

        skyboxBack->SetPosition(glm::vec3(-100, 100, 100));
        skyboxBack->SetScale(glm::vec3(skyboxScale, skyboxScale, skyboxScale));
        skyboxBack->Rotate(glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0)));
       
        skyboxLeft->SetPosition(glm::vec3(-100, 100, -100));
        skyboxLeft->SetScale(glm::vec3(skyboxScale, skyboxScale, skyboxScale));
        skyboxLeft->Rotate(glm::angleAxis(glm::radians(-90.0f), glm::vec3(0.0, 0.0, 1.0)));

        skyboxRight->SetPosition(glm::vec3(100, 100, -100));
        skyboxRight->SetScale(glm::vec3(skyboxScale, skyboxScale, skyboxScale));
        skyboxRight->Rotate(glm::angleAxis(glm::radians(-90.0f), glm::vec3(0.0, 0.0, 1.0)));
        

        game::SceneNode* map = CreateInstance("MapInstance1", "GameMapMesh", "TexturedMaterial", "GrassTexture");

    }

    void Game::MainLoop(void) {

        float previousVer = 0.0f;
        float horizontalAngle = 0.0f;
        float verticalAngle = 0.0f;
        

        float mouseSpeed = 0.01f;

        double xpos, ypos;
       
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
            if (animating_ && !usingUI && !isDead && !game_is_over) {
                static double last_time = 0;
                double current_time = glfwGetTime();
                if ((current_time - last_time) > 0.01) {
                    /*
                    =========================================================
                    MAIN LOOP
                    =========================================================
                    */

                    //!/ Camera Movement and Handle
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

                    EnemyMovement(current_time - last_time);
                    CollisionDetection();
                    //scene_.Update();

                    //!/ Grab the map instance
                    SceneNode* node = scene_.GetNode("MapInstance1");


                    last_time = current_time;
                }
            }

            if (isDead) {
                static double deathTime = current_time;
                //deathTime += (current_time - last_time);
                if (current_time - deathTime >= 3.0) { 
                    game_is_over = true;
                    usingUI = true;
                    //printf("We died :(");
                }
            }

            if (!isDead) {
                // Draw the scene
                scene_.Draw(&camera_);
            }
            else {
                //Running these line of code will active the death screen effect
            
                scene_.DrawToTexture(&camera_);
                scene_.DisplayTexture(resman_.GetResource("ScreenSpaceMaterial")->GetResource());
            }
            
            if (usingUI) {
                //Start UI
                //Start a new ImGui frame
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                //Menu Text
                std::string StartupText = "Welcome to HUNGRY MAN";
                std::string PressText = "Press Tab to START";
                ImGui::Text(StartupText.c_str());
                ImGui::Text(PressText.c_str());
                GLuint texture_id = resman_.GetResource("Yum")->GetResource();
                ImGui::Image((void*)(intptr_t)texture_id, ImVec2(300, 300));

                if (game_is_over) {
                    ImGui::EndFrame();
                    ImGui::NewFrame();
                    ImGui::Text("Game Over!");
                    GLuint texture_id = resman_.GetResource("HungryManPic")->GetResource();
                    ImGui::Image((void*)(intptr_t)texture_id, ImVec2(300, 300));

                    std::string scoreText = "Your score was " + std::to_string((int) gameScore.w);
                    
                    ImGui::Text(scoreText.c_str());
                }

                //You can just call Text again to add more text to the GUI
                //ImGui::Text(text.c_str());


                //Render the ImGui frame
                ImGui::Render();
                int display_w, display_h;
                glfwGetFramebufferSize(window_, &display_w, &display_h);
                glViewport(0, 0, display_w, display_h);
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
                
                //End UI
            }
            
            
            // Push buffer drawn in the background onto the display
            glfwSwapBuffers(window_);

            // Update other events like input handling
            glfwPollEvents();

            if (!isDead) {
                horizontalAngle = 0.0f;
                verticalAngle = 0.0f;
            }
            
        }
    }

    void Game::EnemyMovement(float dt) {
        SceneNode* head = scene_.GetNode("HungryHead");
        SceneNode* torso = scene_.GetNode("HungryTorso");
        SceneNode* eyes = scene_.GetNode("HungryEyes");

        //Time Variables
        double current_time = glfwGetTime();
        double changeDirectionInterval = 5.0f; // Time in seconds to change direction
        static double directionChangeTimer = current_time; // Timer for direction change
        // Initial patrol direction
        static glm::vec3 patrolDirection = glm::vec3(cos(glm::radians(dist(rng))), 0.0f, sin(glm::radians(dist(rng))));

        //!/ PATROL state
        if (head->GetState() == 1 || inCabin) {
            float spottingRadius = 5.0f;

            glm::vec3 hungryPosition = head->GetPosition();

            
            if (current_time - directionChangeTimer >= changeDirectionInterval) {
                //New angle should generally be +90 from last
                float newAngle = glm::radians(dist(rng)) + glm::radians(90.0f);
                patrolDirection = glm::vec3(cos(newAngle), 0.0f, sin(newAngle));
                directionChangeTimer = current_time;
            }

            //!/ Calculate the yaw angle for the current patrol direction
            float yaw = std::atan2(patrolDirection.z, patrolDirection.x);

            //!/ Set the head and torso orientation
            head->SetOrientation(glm::angleAxis(yaw - glm::radians(90.0f), glm::vec3(0, -1, 0)));
            torso->SetOrientation(glm::angleAxis(yaw - glm::radians(90.0f), glm::vec3(0, -1, 0)));

            //!/ Calculation and Offset for Hungry
            float newY = 3 * cos((M_PI / fabs(50 * 2 / 3)) * hungryPosition.x);

            //!/ Bounds of his elevation are 0.0 to -3.0f
            if (hungryPosition.x < 50 * 1 / 3) { newY = 0.0f; }
            else if (hungryPosition.x > 50 * 2 / 3) { newY = -3.0f; }
            else if (hungryPosition.z > 50 * 1 / 2) { newY = -3.0f; }

            //!/ Update the y
            hungryPosition.y = newY + 3;

            head->SetPosition(hungryPosition);
            torso->SetPosition(hungryPosition);

            patrolDirection.y = 0.0f; //Ensure no Y movement
            glm::vec3 movement = patrolDirection * (hungry_speed * dt); 

            head->Translate(movement);
            torso->Translate(movement);


            

            if ((glm::distance(camera_.GetPosition(), hungryPosition) < spottingRadius && isHidden == false) && !inCabin) {
                head->SetEnemyState(2);            
            }
        }

        //!/ CHASE state
        else if (head->GetState() == 2) {
            


            //!/ Radius and direction variables
            float chaseRadius = 5.0f;
            glm::vec3 direction = camera_.GetPosition() - head->GetPosition();

            //!/ Calculate the Distances
            float horizontalDistance = std::sqrt(pow(direction.x,2) + pow(direction.z, 2));
            float verticalDistance = direction.y;

            //!/ Compute the pitch and yaw angles
            float pitch = std::atan2(verticalDistance, horizontalDistance);
            float yaw = std::atan2(direction.z, direction.x);

            //!/ Set the head and torso orientation
            head->SetOrientation(glm::angleAxis(yaw - glm::radians(90.0f), glm::vec3(0, -1, 0)));
            torso->SetOrientation(glm::angleAxis(yaw - glm::radians(90.0f), glm::vec3(0, -1, 0)));

            //!/ Grab the position
            glm::vec3 hungryPosition = head->GetPosition();

            //!/ Calculation and Offset for Hungry
            float newY = 3 * cos((M_PI / fabs(50 * 2 / 3)) * hungryPosition.x);
            
            //!/ Bounds of his elevation are 0.0 to -3.0f
            if (hungryPosition.x < 50 * 1 / 3) { newY = 0.0f; }
            else if (hungryPosition.x > 50 * 2 / 3) { newY = -3.0f; }
            else if (hungryPosition.z > 50 * 1 / 2) { newY = -3.0f; }
            
            //!/ Update the y
            hungryPosition.y = newY + 3;

            //!/ Move head and torso concurrently (Head will look at player, and we dont want its rotations to affect the torso.)
            
            if (hungryPosition.y - head->GetPosition().y != 3.0f) {
                head->SetPosition(hungryPosition);
                torso->SetPosition(hungryPosition);

                direction.y = 0.0f;
                direction.x *= (hungry_speed * dt);
                direction.z *= (hungry_speed * dt);

                head->Translate(direction);
                torso->Translate(direction);
            }
            else {
                head->SetEnemyState(1);
            }
            
            //!/ If the player is hiding in a bush.
            if ( (glm::distance(camera_.GetPosition(), hungryPosition) > chaseRadius && isHidden) || inCabin) {
                head->SetEnemyState(1);
            }
        }      

    }

    void Game::CollisionDetection() {

        //!/ Determine current player position
        glm::vec3 playerPosition = camera_.GetPosition();
        
        if (playerPosition.x > 49.0f || playerPosition.x < 1.0f || playerPosition.z > 49.0f || playerPosition.z < 1.0f) {
            camera_.SetPosition(lastPosition);
        }



        //!/ Grabbing wall properties and cabin positions 
        SceneNode* cabinDoor = scene_.GetNode("CabinEntrance");
        glm::vec3 wallStart = cabinDoor->GetPosition();
        glm::vec3 wallEnd = cabinDoor->GetPosition();
        wallEnd.x = cabinDoor->GetPosition().x + 3.3;
        wallStart.x = cabinDoor->GetPosition().x - 3.3;
        wallEnd.z = cabinDoor->GetPosition().z + 6.9;
        wallStart.z = cabinDoor->GetPosition().z - 0.3;

        //!/ True: If within walls of Cabin, False: If outside walls of cabin
        if ( (playerPosition.x > wallStart.x && playerPosition.x < wallEnd.x) && (playerPosition.z > wallStart.z && playerPosition.z < wallEnd.z)) {inCabin = true;}
        else {inCabin = false;}

        // COLLISION CHECK FOR EVERY OTHER OBJECT
        for (auto it = scene_.begin(); it != scene_.end(); ++it) {
            SceneNode* currentObj = *it;

            // Collision for Trees
            if ((currentObj->GetName()).find("TreeTrunk") != std::string::npos) {
                glm::vec3 objPosition = currentObj->GetPosition();
                
                objPosition.y = heightMap[(int)(playerPosition.z + (playerPosition.x * 50))];

                float objRadius = 1.0f; //Needs to be changed per object

                if (glm::distance(playerPosition, objPosition) < objRadius) {
                    //std::cout << "COLLISION with " << currentObj->GetName() << "\n";
                    camera_.SetPosition(lastPosition); //Reset player position
                }
            }

            //!/ Collision for ALL Cabin Walls (Not the Entrance) - THESE SHOULD BE ABSOLUTELY SOLID THE PLAYER CANNOT GO THROUGH THE WINDOW BECAUSE THEY SUCK
            // As the walls are alligned on the X-AXIS, We do it based off the X-Axis.
            if (currentObj->GetName().find("WallWindow") != std::string::npos || currentObj->GetName().find("WallFull") != std::string::npos) {

                glm::vec3 objPosition = currentObj->GetPosition();

                glm::vec3 wallStart = currentObj->GetPosition();
                glm::vec3 wallEnd = currentObj->GetPosition();

                wallEnd.z = objPosition.z + 3.3;
                wallStart.z = objPosition.z - 3.3;

                wallEnd.x = objPosition.x + 0.3;
                wallStart.x = objPosition.x - 0.3;

                float objRadius = 1.0f; //Needs to be changed per object

                if ( (playerPosition.x > wallStart.x && playerPosition.x < wallEnd.x) && (playerPosition.z > wallStart.z && playerPosition.z < wallEnd.z)) {
                    camera_.SetPosition(lastPosition); //Reset player position
                    break;
                }

            }

            //!/ Collision for ALL ENTRANCES (Not the Walls) - There are two entrances, same orientation, so collision for detecting the door should be the same.
            if ((currentObj->GetName()).find("CabinEntrance") != std::string::npos) {

                glm::vec3 objPosition = currentObj->GetPosition();

                glm::vec3 wallStart = currentObj->GetPosition();
                glm::vec3 wallEnd = currentObj->GetPosition();
                glm::vec3 wallFrameStart = currentObj->GetPosition();
                glm::vec3 wallFrameEnd = currentObj->GetPosition();

                wallEnd.x = objPosition.x + 1.65;
                wallStart.x = objPosition.x - 3.3;

                wallFrameStart.x = objPosition.x + 2.2;
                wallFrameEnd.x = objPosition.x + 3.3;

                wallEnd.z = objPosition.z + 0.3;
                wallStart.z = objPosition.z - 0.3;

                float objRadius = 1.0f; //Needs to be changed per object

                if ( ( (playerPosition.x > wallStart.x && playerPosition.x < wallEnd.x) || (playerPosition.x > wallFrameStart.x && playerPosition.x < wallFrameEnd.x)) && (playerPosition.z > wallStart.z && playerPosition.z < wallEnd.z)) {
                    camera_.SetPosition(lastPosition); //Reset player position
                    break;
                }
            }

            //!/ Hiding mechanic
            // Collision for Bushes
            if ((currentObj->GetName()).find("Bush") != std::string::npos) {

                glm::vec3 objPosition = currentObj->GetPosition();
                objPosition.y = heightMap[(int)(playerPosition.z + (playerPosition.x * 50))];
                float objRadius = 1.0f; //Needs to be changed per object

                if (glm::distance(playerPosition, objPosition) < objRadius) {
                    //!/ Check if the player is crouching
                    if (isCrouching) {
                        isHidden = true;
                    }
                    else {
                        isHidden = false;
                    }
                }
            }

            //!/ Collectible collisions
            // Collision for Mushroom
            if ((currentObj->GetName()).find("Mushroom") != std::string::npos) {
                glm::vec3 objPosition = currentObj->GetPosition();
                objPosition.y = heightMap[(int)(playerPosition.z + (playerPosition.x * 50))];
                float objRadius = 1.0f;

                if (glm::distance(playerPosition, objPosition) < objRadius) {
                    //printf("[UPDATE] Mushroom Collected\n");

                    if (gameScore.x == 0) {
                        gameScore.x += 1;
                        scene_.RemoveNode(currentObj->GetName());
                    }
                }
            }

            // Collision for Bees
            if ((currentObj->GetName()).find("Bees") != std::string::npos) {
                glm::vec3 objPosition = currentObj->GetPosition();
                objPosition.y = (heightMap[(int)(playerPosition.z + (playerPosition.x * 50))]) - 1;
                float objRadius = 2;

                //printf("%f\n", glm::distance(playerPosition, objPosition));
                if (glm::distance(playerPosition, objPosition) < objRadius) {
                    //printf("[UPDATE] Bee Collected\n");
                    if (gameScore.y == 0) {
                        gameScore.y += 1;
                        scene_.RemoveNode(currentObj->GetName());
                    }
                }
            }

            // Collision for Nail
            if ((currentObj->GetName()).find("Nail") != std::string::npos) {
                glm::vec3 objPosition = currentObj->GetPosition();
                objPosition.y = heightMap[(int)(playerPosition.z + (playerPosition.x * 50))] + 1;
                float objRadius = 0.8f;

                if (glm::distance(playerPosition, objPosition) < objRadius) {
                    //printf("[UPDATE] Nail Collected\n");

                    if (gameScore.z == 0) {
                        gameScore.z += 1;
                        scene_.RemoveNode(currentObj->GetName());
                    }
                }
            }

            // Collision for Objective Marker
            if ((currentObj->GetName()).find("ObjectiveMarker") != std::string::npos) {
                glm::vec3 objPosition = currentObj->GetPosition();
                objPosition.y = heightMap[(int)(playerPosition.z + (playerPosition.x * 50))] + 1;
                float objRadius = 1.0f;

                if (glm::distance(playerPosition, objPosition) < objRadius) {

                    if (gameScore.x == 1 && gameScore.y == 1 && gameScore.z == 1) {
                        gameScore.x--; gameScore.y--; gameScore.z--;
                        gameScore.w++;

                        hungry_speed += 0.2f;
                        std::cout << "CANDY (" << (int)gameScore.w << ")" << std::endl;
                        CreateCollectibles(1, 1, 1, glm::vec3(10.0, 3.0, 25.0));
                    }
                }
            }

            //!/ Enemy Collision
            // Collision for Hungry Man
            if ((currentObj->GetName()).find("HungryTorso") != std::string::npos) {
                glm::vec3 objPosition = currentObj->GetPosition();
                objPosition.y = heightMap[(int)(playerPosition.z + (playerPosition.x * 50))] - 1;
                float objRadius = 2.0f;

                //printf("%f\n", glm::distance(playerPosition, objPosition));
                if (glm::distance(playerPosition, objPosition) < objRadius) {
                    //std::cout << "I got you!" << std::endl;
                    isDead = true;
                }
            }

        }
    }

    void Game::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

        // Get user data with a pointer to the game class
        void* ptr = glfwGetWindowUserPointer(window);
        Game* game = (Game*)ptr;
        double lastToggleTime = 0.0;
        const double toggleDelay = 0.5;
        glm::vec3 newPosition;

        // Quit game if 'q' is pressed
        if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        //// Stop animation if space bar is pressed
        //if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        //    game->animating_ = (game->animating_ == true) ? false : true;
        //}

        //!/ View control
        float rot_factor(glm::pi<float>() / 180);
        float trans_factor = 0.1;

        float currentHeight = 0.8;

        if (isCrouching) {
            currentHeight = 0.4;
            trans_factor = 0.05;
        }
        else {
            currentHeight = 0.8;
        }

        //!/ WASD movment
        if (key == GLFW_KEY_W && !isDead) {
            //Last position snippet
            lastPosition = game->camera_.GetPosition();

            newPosition = game->camera_.GetPosition() + (game->camera_.GetForward() * trans_factor);
            float groundHeight = game->GetHeightFromMap(newPosition.x, newPosition.z, v_gWidthReal, v_gLengthReal);
            //printf("Ground Height: %f, newPosition.y: %f\n", groundHeight, newPosition.y);
            
            if (groundHeight - lastPosition.y > 2) {
                game->camera_.SetPosition(lastPosition);
                
            }
            else {
                newPosition.y = groundHeight + currentHeight;
                game->camera_.SetPosition(newPosition);
            }
            
        }
        if (key == GLFW_KEY_S && !isDead) {
            //Last position snippet
            lastPosition = game->camera_.GetPosition();

            newPosition = game->camera_.GetPosition() - (game->camera_.GetForward() * trans_factor);
            float groundHeight = game->GetHeightFromMap(newPosition.x, newPosition.z, v_gWidthReal, v_gLengthReal);
            //printf("Ground Height: %f, newPosition.y: %f\n", groundHeight, newPosition.y);

            if (groundHeight - lastPosition.y > 2) {
                game->camera_.SetPosition(lastPosition);

            }
            else {
                newPosition.y = groundHeight + currentHeight;
                game->camera_.SetPosition(newPosition);
            }
            
        }
        if (key == GLFW_KEY_A && !isDead) {
            //Last position snippet
            lastPosition = game->camera_.GetPosition();

            newPosition = game->camera_.GetPosition() - (game->camera_.GetSide() * trans_factor);
            float groundHeight = game->GetHeightFromMap(newPosition.x, newPosition.z, v_gWidthReal, v_gLengthReal);
            //printf("Ground Height: %f, newPosition.y: %f\n", groundHeight, newPosition.y);

            if (groundHeight - lastPosition.y > 2) {
                game->camera_.SetPosition(lastPosition);

            }
            else {
                newPosition.y = groundHeight + currentHeight;
                game->camera_.SetPosition(newPosition);
            }
        }

        if (key == GLFW_KEY_D && !isDead) {
            //Last position snippet
            lastPosition = game->camera_.GetPosition();

            newPosition = game->camera_.GetPosition() + (game->camera_.GetSide() * trans_factor);
            float groundHeight = game->GetHeightFromMap(newPosition.x, newPosition.z, v_gWidthReal, v_gLengthReal);
            //printf("Ground Height: %f, newPosition.y: %f\n", groundHeight, newPosition.y);

            if (groundHeight - lastPosition.y > 2) {
                game->camera_.SetPosition(lastPosition);

            }
            else {
                newPosition.y = groundHeight + currentHeight;
                game->camera_.SetPosition(newPosition);
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
                //printf("usingMouseCamera: %d\n", usingMouseCamera);
                lastToggleTime = currentTime; 
            }
        }
        
        //!/ Space button to create upwards movment
        if (key == GLFW_KEY_SPACE) {
            /*
            if (game->camera_.GetPosition().y - 1.0 >= -1.5) {
                game->camera_.Translate(game->camera_.GetUp() * trans_factor);
            }
            */

        }

        //!/ "C" will toggle stance (Crouch vs Stand)
        if (key == GLFW_KEY_C && action == GLFW_PRESS) {
            double currentTime = glfwGetTime();
            if (currentTime - lastToggleTime > toggleDelay) {
                if (isCrouching) {
                    newPosition = game->camera_.GetPosition();
                    newPosition.y += 0.4;
                    game->camera_.SetPosition(newPosition);
                    isCrouching = false;
                }
                else {
                    newPosition = game->camera_.GetPosition();
                    newPosition.y -= 0.4;
                    game->camera_.SetPosition(newPosition);
                    isCrouching = true;
                }
                lastToggleTime = currentTime;
            }
        }

        //!/ Tab will allow the game to start
        if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
            usingUI = false;
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

    //!/ This function creates the enemy
    //! It takes the location designated for the enemy
    void Game::CreateHungry(glm::vec3 location) {
        
        //!/ Make all limbs of hungry man
        game::SceneNode* head = CreateInstance("HungryHead", "HungryHead", "TexturedMaterial", "HungrySkin"); head->Scale(glm::vec3(0.5, 0.5, 0.5));
        game::SceneNode* eyes = CreateInstance("HungryEyes", "HungryEyes", "TexturedMaterial", "HungryEyesText", head); eyes->Scale(glm::vec3(0.5, 0.5, 0.5));
        game::SceneNode* tongue = CreateInstance("HungryTongue", "HungryTongue", "TexturedMaterial", "HungryTongueText", head); tongue->Scale(glm::vec3(0.5, 0.5, 0.5));
        game::SceneNode* torso = CreateInstance("HungryTorso", "HungryTorso", "TexturedMaterial", "HungrySkin"); torso->Scale(glm::vec3(0.5, 0.5, 0.5));
        game::SceneNode* lArm = CreateInstance("HungryLArm", "HungryLArm", "TexturedMaterial", "HungrySkin", torso); lArm->Scale(glm::vec3(0.5, 0.5, 0.5));
        game::SceneNode* rArm = CreateInstance("HungryRArm", "HungryRArm", "TexturedMaterial", "HungrySkin", torso); rArm->Scale(glm::vec3(0.5, 0.5, 0.5));
        game::SceneNode* lLeg = CreateInstance("HungrylLeg", "HungryLLeg", "TexturedMaterial", "HungrySkin", torso); lLeg->Scale(glm::vec3(0.5, 0.5, 0.5));
        game::SceneNode* rLeg = CreateInstance("HungryrLeg", "HungryRLeg", "TexturedMaterial", "HungrySkin", torso); rLeg->Scale(glm::vec3(0.5, 0.5, 0.5));

        //!/ Set the position for both the head and torso
        head->SetPosition(glm::vec3(30.0f, 0.0, 30.0f));
        torso->SetPosition(glm::vec3(30.0f, 0.0, 30.0f));

        //!/ 0 is for non-enemy, 1 is for patrol, 2 is for chase. 
        head->SetEnemyState(1);

    }

    //!/ Function to create the cabin
    //! This function takes the cabin location
    void Game::CreateCabin(glm::vec3 location) {

        //!/ Grab the locations
        float location_x = location.x;
        float location_y = location.y;
        float location_z = location.z;

        //!/ A wall is of ~6.6 length, and ~1.7 height. - Gabe

        //! Entrances
        game::SceneNode* wallEntrance = CreateInstance("CabinEntrance", "WallDoor", "TexturedMaterial", "TreeBark");
        game::SceneNode* wallEntrance2 = CreateInstance("CabinEntrance2", "WallDoor", "TexturedMaterial", "TreeBark");

        //! Roofs
        game::SceneNode* wallRoof = CreateInstance("WallRoof", "WallRoof", "TexturedMaterial", "TreeBark");
        game::SceneNode* wallRoof2 = CreateInstance("WallRoof2", "WallRoof", "TexturedMaterial", "TreeBark");
        game::SceneNode* roofMain = CreateInstance("Roof", "RoofMain", "TexturedMaterial", "TreeBark");

        //! Walls and Floors
        game::SceneNode* wallWindow = CreateInstance("WallWindow", "WallWindow", "TexturedMaterial");
        game::SceneNode* wallFull = CreateInstance("WallFull", "WallFull", "TexturedMaterial");
        game::SceneNode* floor = CreateInstance("Floor", "WallFull", "TexturedMaterial");
        
        game::SceneNode* objectiveMarker = CreateInstance("ObjectiveMarker", "SphereParticles", "ObjectiveMaterial", "HungryEyesText");
        objectiveMarker->SetPosition(glm::vec3(location_x, location_y, location_z + 3.3f));

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

    //!/ Function to create trees and bushes
    //! This function takes the number of trees, number of bushes and the cabin location
    void Game::CreateProps(int treeNum, int bushNum, glm::vec3 cabin_location) {

        //!/ TREE CREATION
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
                std::uniform_int_distribution<int> x_placementDist(0, 49);
                std::uniform_int_distribution<int> z_placementDist(0, 49);

                std::uniform_int_distribution<int> heightDist(-1, 0);
                std::uniform_int_distribution<int> angleDist(-10, 10);

                int random_x = x_placementDist(gen);
                int random_z = z_placementDist(gen);
                int random_ang = angleDist(gen);

                if (!((random_x < cabin_location.x + 20 && random_x > cabin_location.x - 20) && (random_z < cabin_location.z + 20 && random_z > cabin_location.z - 20))) {
                    int random_y = heightDist(gen) + heightMap[random_z + (random_x * 50)];

                    locationFound = true;
                    treeTrunk->SetPosition(glm::vec3(random_x, random_y, random_z));
                    treeTrunk->Rotate(glm::angleAxis((float)random_ang, glm::vec3(0.0, 1.0, 0.0)));

                    treeTop->SetPosition(glm::vec3(random_x, random_y, random_z));
                    treeTop->Rotate(glm::angleAxis(glm::radians((float)random_ang), glm::vec3(0.0, 1.0, 0.0)));
                }
            }
        }

        //!/ BUSH CREATION
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
                std::uniform_int_distribution<int> placementDist(0, 49);
                std::uniform_int_distribution<int> angleDist(0, 49);

                int random_x = placementDist(gen);
                int random_z = placementDist(gen);

                int random_ang = angleDist(gen);
                if (!((random_x < cabin_location.x + 5 && random_x > cabin_location.x - 15) && (random_z < cabin_location.z + 5 && random_z > cabin_location.z - 15))) {
                    locationFound = true;
                    bush->SetPosition(glm::vec3(random_x, heightMap[random_z + (random_x * 50)]-0.4, random_z));
                    bush->SetScale(glm::vec3(0.7, 0.7, 0.7));
                    bush->Rotate(glm::angleAxis((float)random_ang, glm::vec3(0.0, 1.0, 0.0)));
                }
            }
        }
    }

    //!/ Function to create mushroooms, bees and nails
    //! This function takes the number of mushrooms, number of bees, number of nails and the cabin location
    void Game::CreateCollectibles(int mushNum, int beeNum, int nailNum, glm::vec3 cabin_location) {

        //!/ MUSHROOM CREATION
        for (int i = 0; i < mushNum; ++i) {
            std::stringstream ss;
            ss << i;
            std::string index = ss.str();
            std::string name = "Mushroom" + index;

            game::SceneNode* mushroom = CreateInstance(name, "Mushroom", "TexturedMaterial", "MushroomTexture");

            bool locationFound = false;

            while (!locationFound) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> x_placementDist(0, 49);
                std::uniform_int_distribution<int> z_placementDist(0, 49);
                std::uniform_int_distribution<int> angleDist(-10, 10);

                int random_x = x_placementDist(gen);
                int random_z = z_placementDist(gen);
                int random_ang = angleDist(gen);

                if (!((random_x < cabin_location.x + 20 && random_x > cabin_location.x - 20) && (random_z < cabin_location.z + 20 && random_z > cabin_location.z - 20))) {
                    int random_y = heightMap[random_z + (random_x * 50)];

                    locationFound = true;
                    mushroom->SetPosition(glm::vec3(random_x, random_y, random_z));
                    mushroom->Rotate(glm::angleAxis((float)random_ang, glm::vec3(0.0, 1.0, 0.0)));
                    mushroom->Scale(glm::vec3(0.3, 0.3, 0.3));
                }
            }
        }

        //!/ BEE CREATION
        for (int i = 0; i < beeNum; ++i) {
            std::stringstream ss;
            ss << i;
            std::string index = ss.str();
            std::string name = "Bees" + index;

            game::SceneNode* bee = CreateInstance(name, "BeeParticles", "SwarmMaterial");
            bool locationFound = false;

            while (!locationFound) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> x_placementDist(0, 49);
                std::uniform_int_distribution<int> z_placementDist(0, 49);

                int random_x = x_placementDist(gen);
                int random_z = z_placementDist(gen);

                if (!((random_x < cabin_location.x + 20 && random_x > cabin_location.x - 20) && (random_z < cabin_location.z + 20 && random_z > cabin_location.z - 20))) {
                    int random_y = heightMap[random_z + (random_x * 50)];
                    locationFound = true;
                    bee->SetPosition(glm::vec3(random_x, random_y + 2.0f, random_z));
                }
            }
        }

        //!/ NAIL CREATION
        for (int i = 0; i < nailNum; ++i) {
            std::stringstream ss;
            ss << i;
            std::string index = ss.str();
            std::string name = "Nail" + index;

            game::SceneNode* nail = CreateInstance(name, "Nail", "TexturedMaterial", "NailTexture");

            bool locationFound = false;

            while (!locationFound) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> x_placementDist(0, 49);
                std::uniform_int_distribution<int> z_placementDist(0, 49);

                std::uniform_int_distribution<int> angleDist(-10, 10);

                int random_x = x_placementDist(gen);
                int random_z = z_placementDist(gen);
                int random_ang = angleDist(gen);

                if (!((random_x < cabin_location.x + 20 && random_x > cabin_location.x - 20) && (random_z < cabin_location.z + 20 && random_z > cabin_location.z - 20))) {
                    int random_y = heightMap[random_z + (random_x * 50)];

                    locationFound = true;
                    nail->SetPosition(glm::vec3(random_x, random_y, random_z));
                    nail->Rotate(glm::angleAxis((float)random_ang, glm::vec3(0.0, 1.0, 0.0)));
                    nail->Scale(glm::vec3(0.3, 0.3, 0.3));
                }
            }
        }
    }

    // CreateInstance function
    SceneNode* Game::CreateInstance(std::string entity_name, std::string object_name, std::string material_name, std::string texture_name, SceneNode* parent) {

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

        SceneNode* scn = scene_.CreateNode(entity_name, geom, mat, tex, parent);
        return scn;
    }

    //!/ Create the height map
    //! This function uses these parameters, Number of Quads, Crater Depth, Crater Rad, Crater Position
    //! This function could be easily changed to include number of craters to allow for more craters to be added.
    GLfloat* Game::CreateHeightMap(int v_gWidth, int v_gLength, float hillHeight) {

        //!/ Height Array
        GLfloat* vertexHeight = new GLfloat[v_gWidth * v_gLength];

        //printf("HEIGHT MAP:\n");
        for (int heightX = 0; heightX < v_gWidth; heightX++) {
            for (int heightZ = 0; heightZ < v_gLength; heightZ++) {

                //!/ If statement to divide the map into 3 sections, hill, slope and field
                if (heightX <= (v_gWidth / 3)) {
					vertexHeight[heightZ + (heightX * v_gLength)] = 3.0;
					//printf("%.2f  ", hillHeight);
				}
				else if (heightX <= (v_gWidth * 2 / 3)) {
					if (heightZ <= (v_gLength * 1 / 2)) {
						float newY = hillHeight * cos((M_PI / fabs(v_gWidth * 2 / 3)) * heightX);

						vertexHeight[heightZ + (heightX * v_gLength)] = hillHeight + newY;
						//printf("%.2f  ", hillHeight + newY);
					}
					else {
						vertexHeight[heightZ + (heightX * v_gLength)] = 0.0;
						//printf("%.2f  ", 0.0f);
					}
				}
				else {
					vertexHeight[heightZ + (heightX * v_gLength)] = 0.0;
					//printf("%.2f  ", 0.0f);
				}

            }
            //printf("\n");
        }

        return vertexHeight;
    }

    //!/ Height get function
    float Game::GetHeightFromMap(float x, float z, int v_gWidth, int v_gLength) {
        //Keep x and y in bounds
        if (x < 0 || x >= v_gWidth || z < 0 || z >= v_gLength) return 0.0f; 

        //Translate world coordinates to height map indices
        int mapX = static_cast<int>(x);
        int mapZ = static_cast<int>(z);

        //Get height from height map
        return heightMap[mapZ + (mapX * v_gLength)];
    }

} // namespace game
