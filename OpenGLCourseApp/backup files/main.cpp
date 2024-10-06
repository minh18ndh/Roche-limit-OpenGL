#define SDL_MAIN_HANDLED
#define STB_IMAGE_IMPLEMENTATION

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

DirectionalLight dLight(1.0f, 1.0f, 1.0f, 0.5f, 0.8f, 1.0f);

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
GLfloat rotationAngle = 0.0f;

// Initialize start and end positions for the moon
glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 endPosition = glm::vec3(0.0f, 0.0f, 0.0f);
float journeyDuration = 15.0f; // Duration of the journey (s)
float elapsedTime = 0.0f; // Time since started

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

// Music data
Mix_Music* music1 = nullptr;
Mix_Music* music2 = nullptr;
Mix_Music* currentMusic = nullptr;

bool mouseButtonPressed = false;

void CreateObjects()
{
    unsigned int indices0[] = {
        0, 1, 2  // Indices for a single triangle
    };

    GLfloat vertices0[] = {
        //  x        y        z     u    v   nx   ny   nz
        0.0f,      1.0f,   0.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // Top vertex (0)
       -0.866f,   -0.5f,   0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // Bottom left vertex (1)
        0.866f,   -0.5f,   0.0f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f   // Bottom right vertex (2)
    };

    Mesh* obj0 = new Mesh();
    obj0->CreateMesh(vertices0, indices0, 24, 3);
    meshList.push_back(obj0);

    /////////////////////////////////  Create the sphere  ////////////////////////////////////////////
    sphere = new Sphere(5.0f, 144, 72); // radius, sectors, stacks
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
    ring = new Ring(4.8f, 5.0f, 288); // inner radius, outer radius, sectors
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
    ///////////////////////////////////  end of creating ring  /////////////////////////////////////////////

    glBindVertexArray(0);
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

    // Play music1 by default
    currentMusic = music1;
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

    if (mouseButtonPressed)
    {
        camera.mouseControl(xoffset, yoffset);
    }
}

int main()
{
    ///////////////////////////////////////////  Roche Calculation  /////////////////////////////////////////////////////
    // Uranus data
    float UranusRadius = 25559.0f;
    std::cout << "The radius of Uranus is: " << UranusRadius << " km" << std::endl;
    float UranusMass = 8.6810e25f;
    std::cout << "The mass of Uranus is: " << UranusMass << " kg" << std::endl;

    // Moon data
    float moonRadius;
    //std::cout << "Enter the radius for the moon (km): ";
    //std::cin >> moonRadius;
    moonRadius = 1000;
    std::cout << "The radius of moon is: " << moonRadius << " km" << std::endl;

    float massRatio;
    float moonMass;
    //std::cout << "Enter the (mass of the moon / mass of Uranus) ratio: ";
    //std::cin >> massRatio;
    massRatio = 0.000005f;
    moonMass = massRatio * UranusMass;
    std::cout << "The mass of moon is: " << moonMass << " kg" << std::endl;

    // Roche limit calculation
    float RocheRadius;
    RocheRadius = moonRadius * std::pow((2 / massRatio), (1.0f / 3.0f));
    std::cout << "The Roche limit is approximately: " << RocheRadius << " km" << std::endl;

    // Radii ratio calculation for moon radius
    float moonUranusRadiusRatio;
    moonUranusRadiusRatio = moonRadius / UranusRadius;
    std::cout << "The radii ratio (moon radius / Uranus radius) is approximately: " << moonUranusRadiusRatio << std::endl;

    // Radii ratio calculation for ring scaling
    float RocheUranusRadiusRatio;
    RocheUranusRadiusRatio = RocheRadius / UranusRadius;
    std::cout << "The radii ratio (Roche radius / Uranus radius) is approximately: " << RocheUranusRadiusRatio << std::endl;
    ///////////////////////////////////////////  end of Roche calculation  //////////////////////////////////////////////////////

    // Initialize SDL2_mixer
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return -1;
    }

    mainWindow = Window(1440, 1080);
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

    camera = Camera(glm::vec3(-3.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f, 5.0f, 0.2f);

    // Set camera sensitivity
    camera.setSensitivity(0.1f); // Value to change drag speed

    ////////////////////////////////////////  Load textures  /////////////////////////////////////////
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
    //////////////////////////////////  end of loading textures  /////////////////////////////////////

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
        if (mainWindow.getsKeys()[GLFW_KEY_M]) {
            SwitchMusic();
        }

        // Reset moon position when press R
        if (mainWindow.getsKeys()[GLFW_KEY_R]) {
            elapsedTime = 0.0f;
        }

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

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Triangle at origin
        glm::mat4 model0 = glm::mat4(1.0f); // Initialize the model matrix
        model0 = glm::translate(model0, glm::vec3(0.0f, 0.0f, 0.0f)); // Translate the model matrix
        model0 = glm::scale(model0, glm::vec3(2.0f, 2.0f, 2.0f)); // Scale the model matrix
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model0)); // Send the model matrix to the shader
        uranusTexture.UseTexture(); // Use the texture
        meshList[0]->RenderMesh(); // Render the mesh

        // Triangle at z+30
        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(0.0f, 0.0f, 30.0f));
        model1 = glm::scale(model1, glm::vec3(2.0f, 2.0f, 2.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model1));
        catTexture.UseTexture();
        meshList[0]->RenderMesh();

        // Triangle at x+30
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(30.0f, 0.0f, 0.0f));
        model2 = glm::scale(model2, glm::vec3(2.0f, 2.0f, 2.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model2));
        titaniaTexture.UseTexture();
        meshList[0]->RenderMesh();
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        //////////////////////////////////////////  Rendering spheres  ///////////////////////////////////////////////////////
        // Data for travelling distance and breaking point of the moon
        float RocheLimitPosition;
        RocheLimitPosition = 10.0f - (RocheUranusRadiusRatio * 5.0f) - (5.0f * moonUranusRadiusRatio * 5.0f);
        float distanceToTravel;
        distanceToTravel = RocheLimitPosition + 30.0f;
        
        // Start and end positions of the moon
        glm::vec3 startPosition = glm::vec3(10.0f, -2.0f, -30.0f);
        glm::vec3 endPosition = glm::vec3(10.0f, -2.0f, RocheLimitPosition);
        elapsedTime += deltaTime;
        float journeyProgress = elapsedTime / journeyDuration; // Percentage of journey travelled
        journeyProgress = glm::min(journeyProgress, 1.0f); // journeyProgress must not exceed 1.0

        // Interpolate position for moon
        glm::vec3 currentPosition = glm::mix(startPosition, endPosition, journeyProgress);
        rotationAngle += 10.0f * deltaTime; // Angular velocity about y-axis

        // Data for the moon deformity
        float currentZ = currentPosition.z;
        float horizontalBulgeCoeff;
        horizontalBulgeCoeff = 0.95f - (((currentZ + 30.0f) / distanceToTravel) * (0.95f - 0.7f));
        float yCompressionCoeff;
        yCompressionCoeff = std::pow(horizontalBulgeCoeff, 2.0f);

        // Render sphere 1 (moon) with interpolated position
        glm::mat4 sphereModel1 = glm::mat4(1.0f);
        sphereModel1 = glm::translate(sphereModel1, currentPosition);
        sphereModel1 = glm::rotate(sphereModel1, glm::radians(rotationAngle * 3), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis
        sphereModel1 = glm::scale(sphereModel1, glm::vec3(moonUranusRadiusRatio * 5.0f / horizontalBulgeCoeff, moonUranusRadiusRatio * 5.0f * yCompressionCoeff, 
                                                            moonUranusRadiusRatio * 5.0f / horizontalBulgeCoeff));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(sphereModel1));
        titaniaTexture.UseTexture();
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphere->getIndices().size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Render sphere 2 (Uranus)
        glm::mat4 sphereModel2 = glm::mat4(1.0f);
        sphereModel2 = glm::translate(sphereModel2, glm::vec3(10.0f, -2.0f, 10.0f));
        sphereModel2 = glm::rotate(sphereModel2, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        sphereModel2 = glm::scale(sphereModel2, glm::vec3(1.0f, 0.9f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(sphereModel2));
        uranusTexture.UseTexture();
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphere->getIndices().size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Render big sphere (environment)
        glm::mat4 sphereModel3 = glm::mat4(1.0f);
        sphereModel3 = glm::translate(sphereModel3, camera.getCameraPosition()); // Position the sphere at the camera's position
        sphereModel3 = glm::scale(sphereModel3, glm::vec3(20.0f, 20.0f, 20.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(sphereModel3));
        starTexture.UseTexture();
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphere->getIndices().size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        ////////////////////////////////////  end of rendering spheres  ////////////////////////////////////////////////////
        
        // Render the ring around Uranus
        glm::mat4 ringModel = glm::mat4(1.0f);
        ringModel = glm::translate(ringModel, glm::vec3(10.0f, -2.0f, 10.0f));
        ringModel = glm::rotate(ringModel, glm::radians(rotationAngle * -1), glm::vec3(0.0f, 1.0f, 0.0f));
        ringModel = glm::scale(ringModel, glm::vec3(RocheUranusRadiusRatio, 0.0f, RocheUranusRadiusRatio));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(ringModel));
        ringTexture.UseTexture();
        glBindVertexArray(ringVAO);
        glDrawElements(GL_TRIANGLES, ring->getIndices().size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    // Cleanup
    Mix_FreeMusic(music1);
    Mix_FreeMusic(music2);
    Mix_CloseAudio();
    SDL_Quit();
    delete sphere;

    return 0;
}
