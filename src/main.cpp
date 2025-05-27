#include <glad.h>
#include <glfw3.h>
#include <bits/stdc++.h>
#include <random>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct Car {
    float x, y;
    float speed;
    int direction; // 0 = horizontal, 1 = vertical
};

struct TextureInfo {
    unsigned int id;
    int width;
    int height;
};

std::vector<Car> horizontalCars;
std::vector<Car> verticalCars;

bool horizontalGreen = true;
bool verticalGreen = false;

float simulationSpeed = 1.0f;

// Add key state flags
bool key1Pressed = false;
bool key2Pressed = false;

// Variables for random car generation
std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
std::uniform_real_distribution<float> spawnChanceDist(0.0f, 1.0f);
std::uniform_int_distribution<int> carTypeDist(0, 1);

double lastSpawnTime = glfwGetTime();
double spawnInterval = 0.5; // seconds between spawn attempts
float spawnProbability = 0.7; // probability of spawning a car when interval is met

// Texture IDs
// unsigned int texture1;
// unsigned int texture2;

// Texture Info
TextureInfo texture1Info;
TextureInfo texture2Info;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        horizontalGreen = !horizontalGreen;
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        verticalGreen = !verticalGreen;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        simulationSpeed += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && simulationSpeed > 0.01f)
        simulationSpeed -= 0.01f;
}

void drawRectangle(float x, float y, float width, float height, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
    glEnd();
}

void renderScene() {
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw roads
    drawRectangle(-1.0f, -0.1f, 2.0f, 0.2f, 0.2f, 0.2f, 0.2f); // Horizontal
    drawRectangle(-0.1f, -1.0f, 0.2f, 2.0f, 0.2f, 0.2f, 0.2f); // Vertical

    // Draw traffic lights (larger)
    drawRectangle(0.3f, 0.1f, 0.1f, 0.1f, horizontalGreen ? 0.0f : 1.0f, horizontalGreen ? 1.0f : 0.0f, 0.0f); // Horizontal light
    drawRectangle(0.1f, 0.3f, 0.1f, 0.1f, verticalGreen ? 0.0f : 1.0f, verticalGreen ? 1.0f : 0.0f, 0.0f); // Vertical light

    // Draw horizontal cars (larger)
    for (auto& car : horizontalCars) {
        drawRectangle(car.x, car.y, 0.15f, 0.1f, 1.0f, 0.0f, 0.0f);
    }

    // Draw vertical cars (larger)
    for (auto& car : verticalCars) {
        drawRectangle(car.x, car.y, 0.1f, 0.15f, 0.0f, 0.0f, 1.0f);
    }

    // Draw textures to the right of signal lights (maintaining aspect ratio)
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f); // Set color to white for textured quads

    // Draw texture 1 right of horizontal light (0.3f, 0.1f) size 0.1x0.1
    glBindTexture(GL_TEXTURE_2D, texture1Info.id);
    float aspectRatio1 = (float)texture1Info.width / texture1Info.height;
    float quadWidth1, quadHeight1;
    float targetSize = 0.2f; // Target size for the larger dimension

    if (texture1Info.width > texture1Info.height) {
        quadWidth1 = targetSize;
        quadHeight1 = targetSize / aspectRatio1;
    } else {
        quadHeight1 = targetSize;
        quadWidth1 = targetSize * aspectRatio1;
    }

    // Position centered vertically to the right of the horizontal light
    float lightRight1 = 0.3f + 0.1f; // Right edge of horizontal light
    float lightCenterY1 = 0.1f + 0.1f / 2.0f; // Vertical center of horizontal light
    float quadBottomY1 = lightCenterY1 - quadHeight1 / 2.0f;

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(lightRight1, quadBottomY1); // Bottom-left
        glTexCoord2f(1.0f, 1.0f); glVertex2f(lightRight1 + quadWidth1, quadBottomY1); // Bottom-right
        glTexCoord2f(1.0f, 0.0f); glVertex2f(lightRight1 + quadWidth1, quadBottomY1 + quadHeight1); // Top-right
        glTexCoord2f(0.0f, 0.0f); glVertex2f(lightRight1, quadBottomY1 + quadHeight1); // Top-left
    glEnd();

    // Draw texture 2 right of vertical light (0.1f, 0.3f) size 0.1x0.1
    glBindTexture(GL_TEXTURE_2D, texture2Info.id);
    float aspectRatio2 = (float)texture2Info.width / texture2Info.height;
    float quadWidth2, quadHeight2;

    if (texture2Info.width > texture2Info.height) {
        quadWidth2 = targetSize;
        quadHeight2 = targetSize / aspectRatio2;
    } else {
        quadHeight2 = targetSize;
        quadWidth2 = targetSize * aspectRatio2;
    }

     // Position centered vertically to the right of the vertical light
    float lightRight2 = 0.1f + 0.1f; // Right edge of vertical light
    float lightCenterY2 = 0.3f + 0.1f / 2.0f; // Vertical center of vertical light
    float quadBottomY2 = lightCenterY2 - quadHeight2 / 2.0f;

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(lightRight2, quadBottomY2); // Bottom-left
        glTexCoord2f(1.0f, 1.0f); glVertex2f(lightRight2 + quadWidth2, quadBottomY2); // Bottom-right
        glTexCoord2f(1.0f, 0.0f); glVertex2f(lightRight2 + quadWidth2, quadBottomY2 + quadHeight2); // Top-right
        glTexCoord2f(0.0f, 0.0f); glVertex2f(lightRight2, quadBottomY2 + quadHeight2); // Top-left
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void updateCars() {
    // Update horizontal cars
    for (size_t i = 0; i < horizontalCars.size(); ++i) {
        bool canMove = true;
        // Check for collision with the car ahead
        if (i > 0) {
            // Assuming cars are sorted by position for simplicity
            // If not sorted, a more complex check is needed
            if (horizontalCars[i].x + 0.15f >= horizontalCars[i-1].x - 0.05f) { // 0.05f is the desired gap
                canMove = false;
            }
        }

        // Check for traffic light
        if (!horizontalGreen && horizontalCars[i].x + 0.15f >= -0.1f && horizontalCars[i].x <= 0.1f) {
            canMove = false;
        }

        if (canMove) {
            horizontalCars[i].x += horizontalCars[i].speed * simulationSpeed;
        }
    }

    // Update vertical cars
    for (size_t i = 0; i < verticalCars.size(); ++i) {
        bool canMove = true;
        // Check for collision with the car ahead
        if (i > 0) {
            // Assuming cars are sorted by position for simplicity
            // If not sorted, a more complex check is needed
            if (verticalCars[i].y <= verticalCars[i-1].y + 0.15f + 0.05f) { // 0.05f is the desired gap
                canMove = false;
            }
        }

        // Check for traffic light
        if (!verticalGreen && verticalCars[i].y <= 0.1f && verticalCars[i].y >= -0.1f) {
             canMove = false;
        }

        if (canMove) {
            verticalCars[i].y -= verticalCars[i].speed * simulationSpeed;
        }
    }
}

// Function to load a texture (will be implemented next)
unsigned int loadTexture(const char* filename) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// Modified loadTexture to return TextureInfo
TextureInfo loadTextureInfo(const char* filename) {
    TextureInfo textureInfo = {0, 0, 0};
    glGenTextures(1, &textureInfo.id);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureInfo.id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        textureInfo.width = width;
        textureInfo.height = height;

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
    }

    return textureInfo;
}

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 600, "Traffic Simulation", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Load textures
    // unsigned int texture1 = loadTexture("pic/Traffic-1.png"); // Assuming .png extension, adjust if needed
    // unsigned int texture2 = loadTexture("pic/Traffic-2.png"); // Assuming .png extension, adjust if needed

    texture1Info = loadTextureInfo("pic/Traffic-1.png"); // Assuming .png extension, adjust if needed
    texture2Info = loadTextureInfo("pic/Traffic-2.png"); // Assuming .png extension, adjust if needed

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        updateCars();

        // Random car generation logic
        double currentTime = glfwGetTime();
        if (currentTime - lastSpawnTime >= spawnInterval) {
            lastSpawnTime = currentTime;
            if (spawnChanceDist(rng) < spawnProbability) {
                int carType = carTypeDist(rng);
                if (carType == 0) { // Horizontal car
                    horizontalCars.push_back({-0.95f, -0.05f, 0.005f, 0});
                } else { // Vertical car
                    verticalCars.push_back({-0.05f, 0.95f, 0.005f, 1});
                }
            }
        }

        renderScene();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}