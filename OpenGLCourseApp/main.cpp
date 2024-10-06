#define SDL_MAIN_HANDLED
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "Light.h"
#include "Sphere.h"
#include "Ring.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
Camera camera;

Texture catTexture;
Texture uranusTexture;
Texture titaniaTexture;
Texture starTexture;
Texture ringTexture;
Texture umbrielTexture;
Texture sunTexture;

DirectionalLight dLight(1.0f, 1.0f, 1.0f, 0.95f, 0.8f, 1.0f);

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
GLfloat rotationAngle = 0.0f;

// Initialize start and end positions for the moon
glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 endPosition = glm::vec3(0.0f, 0.0f, 0.0f);
float moonJourneyDuration = 0.0f; // Duration of the journey (s)
float moonElapsedTime = 0.0f; // Time since started
bool moonJourneyStarted = false;
bool moonJourneyEnded = false;

bool renderMoon = true;
bool renderDebris = false;
float debrisElapsedTime = 0.0f;
bool debrisJourneyEnded = false;

bool isPaused = false;

// Vertex Shader
static const char* vShader = "Shaders/shader.vert";

// Fragment Shader
static const char* fShader = "Shaders/shader.frag";

// Sphere data
Sphere* sphere;
GLuint sphereVAO, sphereVBO, sphereEBO;

// Ring data
Ring* ring;
GLuint ringVAO, ringVBO, ringEBO;
Ring* circle;
GLuint circleVAO, circleVBO, circleEBO;

// Music data
Mix_Music* music1 = nullptr;
Mix_Music* music2 = nullptr;
Mix_Music* currentMusic = nullptr;

bool mouseButtonPressed = false;
bool isInteractingWithImGui = false;


std::string LoadTextFile(const char* filepath) {
    std::ifstream file(filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
bool showAboutText = false; 
bool showManual = false;
std::string AboutContent; // String to hold the About content
std::string ManualContent;

void CreateObjects()
{
    unsigned int indices0[] = {
        0, 1, 2  // Indices for a single triangle
    };

    GLfloat vertices0[] = {
        //  x        y        z     u    v   nx   ny   nz
        0.0f,    1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // Top vertex (0)
       -0.866f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // Bottom left vertex (1)
        0.866f, -0.5f, 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f   // Bottom right vertex (2)
    };

    Mesh* obj0 = new Mesh();
    obj0->CreateMesh(vertices0, indices0, 24, 3);
    meshList.push_back(obj0);

    /////////////////////////////////  Create the sphere  ////////////////////////////////////////////
    float sphereBaseRadius = 1.0f;
    sphere = new Sphere(sphereBaseRadius, 144, 72); // radius, sectors, stacks
    std::vector<float> vertices = sphere->getVertices();
    std::vector<unsigned int> indices = sphere->getIndices();

    // Create and bind VAO, VBO, EBO for the sphere
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    //////////////////////////////////  end of creating sphere  ///////////////////////////////////////

    ////////////////////////////////////  Create the ring  ///////////////////////////////////////////
    float ringBaseRadius = sphereBaseRadius;
    ring = new Ring(ringBaseRadius - 0.01f, ringBaseRadius, 288); // inner radius, outer radius, sectors
    vertices = ring->getVertices();
    indices = ring->getIndices();

    // Create and bind VAO, VBO, EBO for the ring
    glGenVertexArrays(1, &ringVAO);
    glGenBuffers(1, &ringVBO);
    glGenBuffers(1, &ringEBO);

    glBindVertexArray(ringVAO);

    glBindBuffer(GL_ARRAY_BUFFER, ringVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ringEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0); // Unbind VAO


    circle = new Ring(ringBaseRadius - 0.0005f, ringBaseRadius, 288);
    vertices = circle->getVertices();
    indices = circle->getIndices();

    // Create and bind VAO, VBO, EBO for the circle
    vertices = circle->getVertices();
    indices = circle->getIndices();

    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glGenBuffers(1, &circleEBO);

    glBindVertexArray(circleVAO);

    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0); // Unbind VAO
    ///////////////////////////////////  end of creating ring  /////////////////////////////////////////////
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

void InitAudio() {
    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize. SDL_mixer Error: %s\n", Mix_GetError());
        return;
    }

    // Load music1
        music1 = Mix_LoadMUS("Music/MoM.wav");
    if (music1 == nullptr) {
        printf("Failed to load music1. SDL_mixer Error: %s\n", Mix_GetError());
        return;
    }

    // Load music2
    music2 = Mix_LoadMUS("Music/CC.wav");
    if (music2 == nullptr) {
        printf("Failed to load music2. SDL_mixer Error: %s\n", Mix_GetError());
        return;
    }

    // Play music2 by default
    currentMusic = music2;
    if (Mix_PlayMusic(currentMusic, -1) == -1) {
        printf("Failed to play music. SDL_mixer Error: %s\n", Mix_GetError());
        return;
    }
}

void SwitchMusic() {
    // Check if music is playing
    if (Mix_PlayingMusic() != 0) {
        // Stop the current music
        Mix_HaltMusic();
    }

    // Switch to the other music track
    if (currentMusic == music1) {
        currentMusic = music2;
    }
    else {
        currentMusic = music1;
    }

    // Play the new music track
    if (Mix_PlayMusic(currentMusic, -1) == -1) {
        printf("Failed to play music. SDL_mixer Error: %s\n", Mix_GetError());
        return;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            mouseButtonPressed = true;
        }
        else if (action == GLFW_RELEASE)
        {
            mouseButtonPressed = false;
        }
    }
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    static double lastX = xpos;
    static double lastY = ypos;

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (mouseButtonPressed && !isInteractingWithImGui)
    {
        camera.mouseControl(xoffset, yoffset);
    }
}

bool previousMKeyState = false;
bool previousPKeyState = false;
bool previousUKeyState = false;
bool previousYKeyState = false;
bool musicSwitched = false;

int main()
{
    ///////////////////////////////////////////  Roche Calculation  /////////////////////////////////////////////
    // Load and print txt files
    std::ifstream file1("Music/roche.txt");
    if (file1.is_open()) {
        std::string line;
        while (std::getline(file1, line)) {
            std::cout << line << std::endl;
        }
        file1.close();
    }
    else {
        std::cerr << "Unable to open file roche.txt" << std::endl;
    }

    std::ifstream file2("Music/instructions.txt");
    if (file2.is_open()) {
        std::string line;
        while (std::getline(file2, line)) {
            std::cout << line << std::endl;
        }
        file2.close();
    }
    else {
        std::cerr << "Unable to open file instructions.txt" << std::endl;
    }

    float sphereBaseRadius = 1.0f;
    float ringBaseRadius = sphereBaseRadius;

    // Uranus data
    float UranusRadius = 25559.0f;
    std::cout << " The radius of Uranus is: " << UranusRadius << " km" << std::endl;
    float UranusMass = 8.681e25f;
    std::cout << " The mass of Uranus is: " << UranusMass << " kg" << std::endl;
    std::cout << " " << std::endl;

    // Moon data
    float moonRadius = 5000.0f; // Default value
    std::string input;
    std::cout << "  The default radius of moon is: " << moonRadius << " km" << std::endl;

    // Radii ratio calculation for moon radius
    float moonUranusRadiusRatio;
    moonUranusRadiusRatio = moonRadius / UranusRadius;

    float massRatio = 0.00005f; // Default value
    float moonMass = massRatio * UranusMass;
    std::cout << "  The default mass of moon is: " << moonMass << " kg" << std::endl;

    // Roche limit calculation
    float RocheRadius;
    RocheRadius = moonRadius * std::pow((2 / massRatio), (1.0f / 3.0f));
    std::cout << "  -> The default Roche limit is approximately: " << RocheRadius << " km" << std::endl;

    // Radii ratio calculation for Umbriel radius
    float UmbrielRadius = 584.7f;
    float UmbrielUranusRadiusRatio;
    UmbrielUranusRadiusRatio = UmbrielRadius / UranusRadius;

    // Scale rate for Umbriel radius
    float UmbrielScaleRate;
    UmbrielScaleRate = 10.0f;

    // Radii ratio calculation for ring scaling
    float RocheUranusRadiusRatio;
    RocheUranusRadiusRatio = RocheRadius / UranusRadius;
    
    // Scale rate for moon radius
    float moonScaleRate;
    moonScaleRate = 1.0f;
    /*std::cout << " Enter the scale rate for the moon's radius (for better visibility) (hit 'Enter' for default = 1): ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        std::stringstream(input) >> moonScaleRate;
    }
    std::cout << "  The scale rate for the moon's radius is: " << moonScaleRate << std::endl;
    std::cout << " " << std::endl;*/

    std::cout << "Initiating..." << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "\r\033[K" << std::flush;
    std::cout << " " << std::endl;

    /*if (RocheRadius > (9.0f * UranusRadius)) {
        std::cout << " No moon will be rendered, since the Roche radius is bigger than the distance from our predefined moon's starting point to the Uranus. In other words, the moon has already been destroyed." << std::endl;
        std::cout << " " << std::endl;
    }*/
    ///////////////////////////////////////////  end of Roche calculation  //////////////////////////////////////////

    // Initialize SDL2_mixer
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return -1;
    }

    mainWindow = Window(2560, 1440);
    mainWindow.Initialise();

    // Make the cursor visible
    glfwSetInputMode(mainWindow.getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Set the mouse button callback
    glfwSetMouseButtonCallback(mainWindow.getGLFWWindow(), mouseButtonCallback);

    // Set the cursor position callback
    glfwSetCursorPosCallback(mainWindow.getGLFWWindow(), cursorPositionCallback);

    CreateObjects();
    CreateShaders();
    InitAudio();

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    // Initialize ImGui for GLFW and OpenGL3
    ImGui_ImplGlfw_InitForOpenGL(mainWindow.getGLFWWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    camera = Camera(glm::vec3(-3.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f, 5.0f, 0.2f);

    // Set camera sensitivity
    camera.setSensitivity(0.1f); // Value to change drag speed

    ////////////////////////////////////////  Load textures  //////////////////////////////////////////////////////
    catTexture = Texture("Textures/cat.jpg");
    catTexture.LoadTexture();

    uranusTexture = Texture("Textures/uranus.jpg");
    uranusTexture.LoadTexture();

    titaniaTexture = Texture("Textures/titania.jpg");
    titaniaTexture.LoadTexture();

    starTexture = Texture("Textures/star.jpg");
    starTexture.LoadTexture();

    ringTexture = Texture("Textures/ring1.jpg");
    ringTexture.LoadTexture();

    umbrielTexture = Texture("Textures/umbriel.jpg");
    umbrielTexture.LoadTexture();

    sunTexture = Texture("Textures/sun.jpg");
    sunTexture.LoadTexture();
    //////////////////////////////////  end of loading textures  ////////////////////////////////////////////////

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformAmbientIntensity = 0,
           uniformAmbientColour = 0, uniformDiffuseIntensity = 0, uniformSpecularIntensity = 0, uniformLightDirection = 0;
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

    // Loop until window closed
    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        // Get + Handle User Input
        glfwPollEvents();

        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        // Switch music when press M
        if (mainWindow.getsKeys()[GLFW_KEY_M] && !previousMKeyState) {
            SwitchMusic();
            std::cout << " Key M is pressed. Music switched manually." << std::endl;
            std::cout << " " << std::endl;
        }
        previousMKeyState = mainWindow.getsKeys()[GLFW_KEY_M];

        // Reset moon position when press R
        if (mainWindow.getsKeys()[GLFW_KEY_R] && moonJourneyStarted) {
            moonElapsedTime = 0.0f;
            moonJourneyStarted = false;
            moonJourneyEnded = false;
            debrisJourneyEnded = false;
            renderMoon = true;
            renderDebris = false;
            std::cout << " Key R is pressed. Moon position reset." << std::endl;
            std::cout << " " << std::endl;
        }

        // Check if the U key is pressed to move the camera
        if (mainWindow.getsKeys()[GLFW_KEY_U] && !previousUKeyState) {
            glm::vec3 newPositionU(1.0f, 4.0f, 16.f);
            camera.setCameraPosition(newPositionU);
            std::cout << " U is pressed. Camera moved to U position: (" << newPositionU.x << ", " << newPositionU.y << ", " << newPositionU.z << ")" << std::endl;
            std::cout << " " << std::endl;
        }
        previousUKeyState = mainWindow.getsKeys()[GLFW_KEY_U];

        // Check if the Y key is pressed to move the camera
        if (mainWindow.getsKeys()[GLFW_KEY_Y] && !previousYKeyState) {
            glm::vec3 newPositionY(6.83451f, -1.73104f, 6.90085f);
            camera.setCameraPosition(newPositionY);
            std::cout << " Y is pressed. Camera moved to Y position: (" << newPositionY.x << ", " << newPositionY.y << ", " << newPositionY.z << ")" << std::endl;
            std::cout << " " << std::endl;
        }
        previousYKeyState = mainWindow.getsKeys()[GLFW_KEY_Y];

        // Clear the window
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderList[0].UseShader();
        uniformModel = shaderList[0].GetModelLocation();
        uniformProjection = shaderList[0].GetProjectionLocation();
        uniformView = shaderList[0].GetViewLocation();
        uniformAmbientColour = shaderList[0].GetAmbientColourLocation();
        uniformAmbientIntensity = shaderList[0].GetAmbientIntensityLocation();
        uniformDiffuseIntensity = shaderList[0].GetDiffuseIntensityLocation();
        uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
        uniformLightDirection = shaderList[0].GetLightDirectionLocation();

        dLight.UseDirLight(uniformAmbientIntensity, uniformAmbientColour,
                             uniformDiffuseIntensity, uniformSpecularIntensity, uniformLightDirection);

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        //////////////////////////////////////  Render triangles  //////////////////////////////////////////////////
        // Triangle
        //glm::mat4 model0 = glm::mat4(1.0f); // Initialize the model matrix
        //model0 = glm::translate(model0, glm::vec3(10.0f, 4.0f, 10.0f)); // Translate the model matrix
        //model0 = glm::scale(model0, glm::vec3(0.3f, 0.3f, 0.3f)); // Scale the model matrix
        //glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model0)); // Send the model matrix to the shader
        //uranusTexture.UseTexture(); // Use the texture
        //meshList[0]->RenderMesh(); // Render the mesh

        //// Triangle at z+30
        //glm::mat4 model1 = glm::mat4(1.0f);
        //model1 = glm::translate(model1, glm::vec3(0.0f, 0.0f, 30.0f));
        //model1 = glm::scale(model1, glm::vec3(1.0f, 1.0f, 1.0f));
        //glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model1));
        //catTexture.UseTexture();
        //meshList[0]->RenderMesh();

        //// Triangle at x+30
        //glm::mat4 model2 = glm::mat4(1.0f);
        //model2 = glm::translate(model2, glm::vec3(30.0f, 0.0f, 0.0f));
        //model2 = glm::scale(model2, glm::vec3(1.0f, 1.0f, 1.0f));
        //glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model2));
        //titaniaTexture.UseTexture();
        //meshList[0]->RenderMesh();
        //////////////////////////////////////  end of rendering triangles  /////////////////////////////////////////
        
        //////////////////////////////////////////  Render spheres  ////////////////////////////////////////////////
        // Data for travelling distance and breaking point of the moon
        glm::vec3 UranusPosition = glm::vec3(10.0f, -2.0f, 10.0f);
        float RocheLimitZPosition;
        RocheLimitZPosition = UranusPosition.z - (RocheUranusRadiusRatio * ringBaseRadius) - (moonScaleRate * moonUranusRadiusRatio * sphereBaseRadius);
        glm::vec3 RocheLimitPosition = glm::vec3(10.0f, -2.0f, RocheLimitZPosition);

        // Start and end positions of the moon
        glm::vec3 startPosition = glm::vec3(10.0f, -2.0f, 0.5f);
        glm::vec3 endPosition = glm::vec3(10.0f, -2.0f, RocheLimitZPosition);

        float moonDistanceToTravel;
        moonDistanceToTravel = RocheLimitZPosition - startPosition.z;
        moonJourneyDuration = moonDistanceToTravel * 1.5f;

        // Initiate moon movement when press I
        if (mainWindow.getsKeys()[GLFW_KEY_I] && !moonJourneyStarted && renderMoon) {
            moonJourneyStarted = true;
            std::cout << " Key I is pressed. Moon movement initiated." << std::endl;
            std::cout << " " << std::endl;
        }

        // Update elapsed time only if the journey has started and not paused
        if (moonJourneyStarted && !isPaused) {
            moonElapsedTime += deltaTime;
        }
        float moonJourneyProgress = moonElapsedTime / moonJourneyDuration; // Percentage of journey travelled
        moonJourneyProgress = glm::min(moonJourneyProgress, 1.0f); // moonJourneyProgress must not exceed 1.0

        // Pause moon movement when press P
        if (mainWindow.getsKeys()[GLFW_KEY_P] && !previousPKeyState && moonJourneyStarted && moonJourneyProgress < 1.0f) {
            if (!isPaused) {
                isPaused = true;
                std::cout << " Key P is pressed. Moon paused." << std::endl;
                std::cout << " " << std::endl;
            }
            else {
                isPaused = false;
                std::cout << " Key P is pressed. Moon resumed." << std::endl;
                std::cout << " " << std::endl;
            }
        }
        previousPKeyState = mainWindow.getsKeys()[GLFW_KEY_P];

        // Check if the moon has reached the end position
        if (moonJourneyProgress >= 1.0f || RocheRadius > (9.0f * UranusRadius)) {
            renderMoon = false;
            renderDebris = true;
        }
        if (!renderDebris) {
            debrisElapsedTime = 0.0f;
        }

        // Interpolate position for moon
        glm::vec3 currentPosition = glm::mix(startPosition, endPosition, moonJourneyProgress);
        rotationAngle += 10.0f * deltaTime; // Angular velocity about y-axis

        // Data for the moon deformity
        float currentZ = currentPosition.z;
        float travelledDistance;
        travelledDistance = currentZ - startPosition.z;

        float horizontalBulgeCoeff;
        if (travelledDistance < (moonDistanceToTravel / 2.0f)) {
            horizontalBulgeCoeff = 0.98f - ((travelledDistance / (moonDistanceToTravel / 2.0f)) * (0.98f - 0.95f)); // Normalize and less deformity change rate in first half of journey
        }
        else {
            horizontalBulgeCoeff = 0.95f - ((travelledDistance - moonDistanceToTravel / 2.0f) / moonDistanceToTravel) * (0.95f - 0.4f);
        } // 0.98, 0.95 and 0.4 are key deformity checkpoints
        float yCompressionCoeff;
        yCompressionCoeff = std::pow(horizontalBulgeCoeff, 2.0f);

        float switchDistance = moonDistanceToTravel * 3.0f / 4.0f;
        // Switch music when traveledDistance reaches 3/4 of moonDistanceToTravel
        if (!musicSwitched && travelledDistance >= switchDistance && renderMoon && RocheRadius > UranusRadius) {
            Mix_PlayMusic(music1, -1);
            std::cout << " Music switched to 'March of Midnight'." << std::endl;
            // std::cout << "The upcoming Roche limit coordinate: (" << currentPosition.x << ", " << currentPosition.y << ", " << RocheLimitZPosition << ")" << std::endl;
            std::cout << " " << std::endl;
            std::cout << " Three-fourths of the moon's journey to the Roche limit has passed, BRACE FOR IMPACT!!" << std::endl;
            std::cout << " " << std::endl;
            musicSwitched = true; // Ensure only switch music once
        }
        if (musicSwitched && travelledDistance < switchDistance) {
            Mix_PlayMusic(music2, -1);
            std::cout << " Music switched to 'Cornfield Chase'." << std::endl;
            std::cout << " " << std::endl;
            musicSwitched = false;
        }

        float ingameCurrentMoonRadius = sphereBaseRadius * moonUranusRadiusRatio * moonScaleRate / horizontalBulgeCoeff;
        if (!moonJourneyEnded && moonJourneyProgress >= 1.0f) {
            if (RocheRadius > UranusRadius) {
                std::cout << " MOON DISINTEGRATED!!!" << std::endl;
                // std::cout << "Moon radius in simulation when reached Roche limit: " << ingameCurrentMoonRadius << "f" << std::endl;
                std::cout << " " << std::endl;
            }
            if (RocheRadius < UranusRadius) {
                std::cout << " MOON COLLIDED WITH URANUS!!!" << std::endl;
                std::cout << " " << std::endl;
                std::cout << " Simulation ended because the Roche radius is smaller than the Uranus' radius. You can press 'R' to simulate again!" << std::endl;
                std::cout << " " << std::endl;
                std::cout << "---------------------------------------------------------- " << std::endl;
                std::cout << " " << std::endl;
            }
            moonJourneyEnded = true;
        }

        // Render sphere 1 (moon) with interpolated position
        if (renderMoon) {
            glm::mat4 sphereModel1 = glm::mat4(1.0f);
            sphereModel1 = glm::translate(sphereModel1, currentPosition);
            sphereModel1 = glm::rotate(sphereModel1, glm::radians(rotationAngle * 2.5f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis
            sphereModel1 = glm::scale(sphereModel1, glm::vec3(moonUranusRadiusRatio * moonScaleRate / horizontalBulgeCoeff, moonUranusRadiusRatio * moonScaleRate * yCompressionCoeff,
                                                        moonUranusRadiusRatio * moonScaleRate / horizontalBulgeCoeff));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(sphereModel1));
            titaniaTexture.UseTexture();
            glBindVertexArray(sphereVAO);
            glDrawElements(GL_TRIANGLES, sphere->getIndices().size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        //////////////////////////////////////////  Render debris  ////////////////////////////////////////////////
        // Render debris
        if (renderDebris && RocheRadius < (9.0f * UranusRadius)) {
            float debrisDistanceToTravel = UranusPosition.z - RocheLimitZPosition;
            float debrisMoonSpeedRatio = debrisDistanceToTravel / moonDistanceToTravel;
            // Update elapsed time only if RocheRadius > UranusRadius
            if (RocheRadius > UranusRadius) {
                debrisElapsedTime += deltaTime;
            }
            float debrisJourneyDuration = moonJourneyDuration * debrisMoonSpeedRatio;
            float debrisJourneyProgress = debrisElapsedTime / debrisJourneyDuration;
            debrisJourneyProgress = glm::min(debrisJourneyProgress, 1.0f);

            // Start and end positions of the debris 1
            glm::vec3 startPosition1 = glm::vec3(10.0f, -2.0f, RocheLimitZPosition - ingameCurrentMoonRadius / 2.0f);
            glm::vec3 endPosition1 = glm::vec3(10.0f + 0.5f, -2.0f + 0.5f, 10.0f);
            glm::vec3 currentPosition1 = glm::mix(startPosition1, endPosition1, debrisJourneyProgress);
            // Render debris 1
            glm::mat4 debrisModel1 = glm::mat4(1.0f);
            debrisModel1 = glm::translate(debrisModel1, currentPosition1);
            debrisModel1 = glm::rotate(debrisModel1, glm::radians(rotationAngle * 4.5f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis
            debrisModel1 = glm::scale(debrisModel1, glm::vec3(moonUranusRadiusRatio * moonScaleRate / horizontalBulgeCoeff / 3.0f, moonUranusRadiusRatio * moonScaleRate * yCompressionCoeff,
                                                    moonUranusRadiusRatio * moonScaleRate / horizontalBulgeCoeff / 3.0f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(debrisModel1));
            titaniaTexture.UseTexture();
            glBindVertexArray(sphereVAO);
            glDrawElements(GL_TRIANGLES, sphere->getIndices().size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            // Start and end positions of the debris 2
            glm::vec3 startPosition2 = glm::vec3(10.0f + ingameCurrentMoonRadius / 2.0f, -2.0f, RocheLimitZPosition - ingameCurrentMoonRadius / 2.0f);
            glm::vec3 endPosition2 = glm::vec3(10.0f + 0.7f, -2.0f - 0.1f, 10.0f);
            glm::vec3 currentPosition2 = glm::mix(startPosition2, endPosition2, debrisJourneyProgress);
            // Render debris 2
            glm::mat4 debrisModel2 = glm::mat4(1.0f);
            debrisModel2 = glm::translate(debrisModel2, currentPosition2);
            debrisModel2 = glm::rotate(debrisModel2, glm::radians(rotationAngle * 4.5f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis
            debrisModel2 = glm::scale(debrisModel2, glm::vec3(moonUranusRadiusRatio * moonScaleRate / horizontalBulgeCoeff / 3.0f, moonUranusRadiusRatio * moonScaleRate * yCompressionCoeff,
                                                    moonUranusRadiusRatio * moonScaleRate / horizontalBulgeCoeff / 3.0f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(debrisModel2));
            titaniaTexture.UseTexture();
            glBindVertexArray(sphereVAO);
            glDrawElements(GL_TRIANGLES, sphere->getIndices().size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            // Start and end positions of the debris 3
            glm::vec3 startPosition3 = glm::vec3(10.0f - ingameCurrentMoonRadius / 2.0f, -2.0f, RocheLimitZPosition - ingameCurrentMoonRadius / 2.0f);
            glm::vec3 endPosition3 = glm::vec3(10.0f - 0.7f, -2.0f + 0.2f, 10.0f);
            glm::vec3 currentPosition3 = glm::mix(startPosition3, endPosition3, debrisJourneyProgress);
            // Render debris 3
            glm::mat4 debrisModel3 = glm::mat4(1.0f);
            debrisModel3 = glm::translate(debrisModel3, currentPosition3);
            debrisModel3 = glm::rotate(debrisModel3, glm::radians(rotationAngle * 4.5f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis
            debrisModel3 = glm::scale(debrisModel3, glm::vec3(moonUranusRadiusRatio * moonScaleRate / horizontalBulgeCoeff / 3.0f, moonUranusRadiusRatio * moonScaleRate * yCompressionCoeff,
                                                    moonUranusRadiusRatio * moonScaleRate / horizontalBulgeCoeff / 3.0f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(debrisModel3));
            titaniaTexture.UseTexture();
            glBindVertexArray(sphereVAO);
            glDrawElements(GL_TRIANGLES, sphere->getIndices().size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            // Start and end positions of the debris 4
            glm::vec3 startPosition4 = glm::vec3(10.0f, -2.0f, RocheLimitZPosition + ingameCurrentMoonRadius / 2.0f);
            glm::vec3 endPosition4 = glm::vec3(10.0f, -2.0f - 0.7f, 10.0f);
            glm::vec3 currentPosition4 = glm::mix(startPosition4, endPosition4, debrisJourneyProgress);
            // Render debris 4
            glm::mat4 debrisModel4 = glm::mat4(1.0f);
            debrisModel4 = glm::translate(debrisModel4, currentPosition4);
            debrisModel4 = glm::rotate(debrisModel4, glm::radians(rotationAngle * 4.5f), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis
            debrisModel4 = glm::scale(debrisModel4, glm::vec3(moonUranusRadiusRatio * moonScaleRate / horizontalBulgeCoeff / 3.0f, moonUranusRadiusRatio * moonScaleRate * yCompressionCoeff,
                                                    moonUranusRadiusRatio * moonScaleRate / horizontalBulgeCoeff / 3.0f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(debrisModel4));
            titaniaTexture.UseTexture();
            glBindVertexArray(sphereVAO);
            glDrawElements(GL_TRIANGLES, sphere->getIndices().size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            // Start and end positions of the debris 5
            glm::vec3 startPosition5 = glm::vec3(10.0f, -2.0f, RocheLimitZPosition);
            glm::vec3 endPosition5 = glm::vec3(10.0f, -2.0f, 10.0f);
            glm::vec3 currentPosition5 = glm::mix(startPosition5, endPosition5, debrisJourneyProgress);
            // Render debris 5
            glm::mat4 debrisModel5 = glm::mat4(1.0f);
            debrisModel5 = glm::translate(debrisModel5, currentPosition5);
            debrisModel5 = glm::rotate(debrisModel5, glm::radians(rotationAngle * 4.5f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
            debrisModel5 = glm::scale(debrisModel5, glm::vec3(moonUranusRadiusRatio * moonScaleRate / horizontalBulgeCoeff / 5.0f, moonUranusRadiusRatio * moonScaleRate * yCompressionCoeff,
                                                    moonUranusRadiusRatio * moonScaleRate / horizontalBulgeCoeff / 5.0f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(debrisModel5));
            titaniaTexture.UseTexture();
            glBindVertexArray(sphereVAO);
            glDrawElements(GL_TRIANGLES, sphere->getIndices().size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            // Calculate debris 6 position along the trajectory
            //float debrisAngle = rotationAngle * 3.0f;
            //glm::vec3 debrisOrbitalPosition = glm::vec3(
            //    10.0f + RocheUranusRadiusRatio * ringBaseRadius * cos(glm::radians(debrisAngle + 90.0f)),
            //    -2.0f,
            //    10.0f + RocheUranusRadiusRatio * ringBaseRadius * sin(glm::radians(debrisAngle + 90.0f))
            //);
            //// Render debris 6
            //glm::mat4 debrisModel6 = glm::mat4(1.0f);
            //debrisModel6 = glm::translate(debrisModel6, debrisOrbitalPosition);
            //debrisModel6 = glm::rotate(debrisModel6, glm::radians(rotationAngle * 2.5f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis
            //debrisModel6 = glm::scale(debrisModel6, glm::vec3(moonUranusRadiusRatio * moonScaleRate / 2.0f, moonUranusRadiusRatio * moonScaleRate / 2.1f,
            //                                        moonUranusRadiusRatio * moonScaleRate/ 2.0f));
            //glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(debrisModel6));
            //titaniaTexture.UseTexture();
            //glBindVertexArray(sphereVAO);
            //glDrawElements(GL_TRIANGLES, sphere->getIndices().size(), GL_UNSIGNED_INT, 0);
            //glBindVertexArray(0);

            if (!debrisJourneyEnded && debrisJourneyProgress >= 1.0f) {
                std::cout << " Simulation ended. You can press 'R' to simulate again!" << std::endl;
                std::cout << " " << std::endl;
                std::cout << "---------------------------------------------------------- " << std::endl;
                std::cout << " " << std::endl;
                debrisJourneyEnded = true;
            }
        }
        //////////////////////////////////////  end of rendering debris  ///////////////////////////////////////////////

        // Render sphere 2 (Uranus)
        glm::mat4 sphereModel2 = glm::mat4(1.0f);
        sphereModel2 = glm::translate(sphereModel2, UranusPosition);
        sphereModel2 = glm::rotate(sphereModel2, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        sphereModel2 = glm::scale(sphereModel2, glm::vec3(1.0f, 0.85f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(sphereModel2));
        uranusTexture.UseTexture();
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphere->getIndices().size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Calculate sphere 3 (Umbriel) position along the trajectory
        float UmbrielOrbitUranusRadiusRatio = 10.407f;
        float angle = rotationAngle * -1.0f; // Orbital motion speed
        // Calculate position along the ring's trajectory
        glm::vec3 orbitalPosition = glm::vec3(
            10.0f + UmbrielOrbitUranusRadiusRatio * ringBaseRadius * cos(glm::radians(angle)),
            -2.0f,
            10.0f + UmbrielOrbitUranusRadiusRatio * ringBaseRadius * sin(glm::radians(angle))
        );
        // Render sphere 3 (Umbriel) with position along the trajectory
        glm::mat4 sphereModel3 = glm::mat4(1.0f);
        sphereModel3 = glm::translate(sphereModel3, orbitalPosition);
        sphereModel3 = glm::rotate(sphereModel3, glm::radians(rotationAngle * 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        sphereModel3 = glm::scale(sphereModel3, glm::vec3(UmbrielUranusRadiusRatio * UmbrielScaleRate, UmbrielUranusRadiusRatio * UmbrielScaleRate * 0.9f,
                                                            UmbrielUranusRadiusRatio * UmbrielScaleRate));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(sphereModel3));
        umbrielTexture.UseTexture();
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphere->getIndices().size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Render big sphere 4 (environment)
        glm::mat4 sphereModel4 = glm::mat4(1.0f);
        sphereModel4 = glm::translate(sphereModel4, camera.getCameraPosition()); // Position the sphere at the camera's position
        sphereModel4 = glm::scale(sphereModel4, glm::vec3(100.0f, 100.0f, 100.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(sphereModel4));
        starTexture.UseTexture();
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphere->getIndices().size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Calculate sphere 5 (Sun) position along the trajectory
        float UranusOrbitSunRadiusRatio = 40.0f;
        float SunAngle = rotationAngle * 0.05f;
        glm::vec3 SunOrbitalPosition = glm::vec3(
            10.0f + UranusOrbitSunRadiusRatio * ringBaseRadius * cos(glm::radians(SunAngle)),
            -2.0f,
            10.0f + UranusOrbitSunRadiusRatio * ringBaseRadius * sin(glm::radians(SunAngle))
        );
        // Render sphere 5 (Sun)
        glm::mat4 sphereModel5 = glm::mat4(1.0f);
        sphereModel5 = glm::translate(sphereModel5, SunOrbitalPosition);
        sphereModel5 = glm::rotate(sphereModel5, glm::radians(rotationAngle * 0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
        sphereModel5 = glm::scale(sphereModel5, glm::vec3(4.0f, 3.9f, 4.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(sphereModel5));
        sunTexture.UseTexture();
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphere->getIndices().size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        ////////////////////////////////////  end of rendering spheres  ///////////////////////////////////////////////
        
        //////////////////////////////////////////  Render rings  ////////////////////////////////////////////////////
        // Render the Roche ring around Uranus
        glm::mat4 RocheRingModel = glm::mat4(1.0f);
        RocheRingModel = glm::translate(RocheRingModel, glm::vec3(10.0f, -2.0f, 10.0f));
        RocheRingModel = glm::scale(RocheRingModel, glm::vec3(RocheUranusRadiusRatio, 0.0f, RocheUranusRadiusRatio));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(RocheRingModel));
        uranusTexture.UseTexture();
        glBindVertexArray(ringVAO);
        glDrawElements(GL_TRIANGLES, ring->getIndices().size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Render the orbital trajectory for Umbriel around Uranus
        glm::mat4 UmbrielRingModel = glm::mat4(1.0f);
        UmbrielRingModel = glm::translate(UmbrielRingModel, glm::vec3(10.0f, -2.0f, 10.0f));
        UmbrielRingModel = glm::scale(UmbrielRingModel, glm::vec3(UmbrielOrbitUranusRadiusRatio, 0.0f, UmbrielOrbitUranusRadiusRatio));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(UmbrielRingModel));
        ringTexture.UseTexture();
        glBindVertexArray(circleVAO);
        glDrawElements(GL_TRIANGLES, circle->getIndices().size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        ////////////////////////////////////  end of rendering rings  ////////////////////////////////////////////////

        ////////////////////////////////////////////  ImGui Rendering  ////////////////////////////////////////////
        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Set up ImGui style
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;
        colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Black button
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f); // Slightly brighter when hovered
        colors[ImGuiCol_ButtonActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f); // Even brighter when active
        colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White text

        // Create a button at the top left of the screen
        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Always);
        ImGui::Begin("About", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

        if (ImGui::Button("About", ImVec2(50, 30))) { // Adjust size here
            // Do something when the button is pressed
            showAboutText = !showAboutText; // Toggle the text display
            showManual = false;
            if (showAboutText) {
                AboutContent = LoadTextFile("Textures/roche_ingame.txt"); // Load the text file
            }
        }
        ImGui::End();

        // Display text if about button was pressed
        if (showAboutText) {
            ImGui::SetNextWindowPos(ImVec2(400, 20), ImGuiCond_Always);
            ImGui::Begin("About Window", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
            ImGui::SetWindowFontScale(1.5f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // White text
            //ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // White background
            ImGui::TextUnformatted(AboutContent.c_str());
            ImGui::PopStyleColor(1); // Revert the colors back to default
            ImGui::End();
        }

        // Create a button for instructions
        ImGui::SetNextWindowPos(ImVec2(90, 20), ImGuiCond_Always);
        ImGui::Begin("Manual", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

        if (ImGui::Button("Manual", ImVec2(50, 30))) { 
            showManual = !showManual; 
            showAboutText = false;
            if (showManual) {
                ManualContent = LoadTextFile("Music/instructions.txt");
            }
        }
        ImGui::End();

        if (showManual) {
            ImGui::SetNextWindowPos(ImVec2(400, 20), ImGuiCond_Always);
            ImGui::Begin("Manual Window", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
            ImGui::SetWindowFontScale(1.5f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            //ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::TextUnformatted(ManualContent.c_str());
            ImGui::PopStyleColor(1); 
            ImGui::End();
        }

        // Create a window for changing the moon radius
        ImGui::SetNextWindowPos(ImVec2(20, 80), ImGuiCond_Always);
        ImGui::Begin("Moon's Radius", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowFontScale(1.3f); // Change the font scale to 1.3x
        if (ImGui::SliderFloat(" km   ", &moonRadius, 1000.0f, 6000.0f, "%.2f")) {
            RocheRadius = moonRadius * std::pow((2 / massRatio), (1.0f / 3.0f));
            RocheUranusRadiusRatio = RocheRadius / UranusRadius;
            moonUranusRadiusRatio = moonRadius / UranusRadius;
        }
        ImGui::End();

        float moonMassDisplay = moonMass / 1.0e20f;
        float moonMoonMassRatio = moonMass / 7.34767e22f;

        // Create a window for changing the moon mass
        ImGui::SetNextWindowPos(ImVec2(20, 150), ImGuiCond_Always);
        ImGui::Begin("Moon's Mass", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowFontScale(1.3f); // Change the font scale to 1.3x

        // Display the moon mass ratio dynamically
        char ratioText[64];
        sprintf(ratioText, "10^20 kg (%.2f mass of Earth's moon)", moonMoonMassRatio);
        ImGui::Text("%s", ratioText);

        if (ImGui::SliderFloat("##MoonMass", &moonMassDisplay, 36.7f, 7347.7f, "%.2f")) {
            moonMass = moonMassDisplay * 1.0e20f;
            moonMoonMassRatio = moonMass / 7.34767e22f;
            massRatio = moonMass / UranusMass;
            RocheRadius = moonRadius * std::pow((2 / massRatio), (1.0f / 3.0f));
            RocheUranusRadiusRatio = RocheRadius / UranusRadius;
            moonUranusRadiusRatio = moonRadius / UranusRadius;
        }

        ImGui::End();

        isInteractingWithImGui = ImGui::IsAnyItemActive(); // Check if any ImGui item is active

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /////////////////////////////////////////////////////////////////////////////////////////////////////////

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    // Cleanup
    Mix_FreeMusic(music1);
    Mix_FreeMusic(music2);
    Mix_CloseAudio();
    SDL_Quit();
  
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete sphere;
    delete ring;

    return 0;
}
