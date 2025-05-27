#include <glad.h>
#include <glfw3.h>
#include <bits/stdc++.h>
#include <random>
#include <chrono>
#include <cmath> // For std::abs

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct Car {
    float x, y;
    float speed; // This will now be maxSpeed
    float currentSpeed; // Add current speed
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

// Define acceleration and deceleration rates
// const float ACCELERATION = 0.0001f; // Increased acceleration
// const float DECELERATION = 0.0004f; // Increased deceleration
// const float BRAKING_DISTANCE_BUFFER = 0.2f; // Increased distance before obstacle to start braking
const float DESIRED_CAR_GAP = 0.05f; // Desired minimum gap between cars

// Add key state flags
bool key1Pressed = false;
bool key2Pressed = false;
// Add a flag for traffic light toggle
bool lightTogglePressed = false;

// Variables for random car generation
std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
std::uniform_real_distribution<float> spawnChanceDist(0.0f, 1.0f);
std::uniform_int_distribution<int> carTypeDist(0, 1);
// Add distribution for random speed
std::uniform_real_distribution<float> carSpeedDist(0.003f, 0.009f); // Range for car speeds

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

    // Removed separate traffic light toggle keys (B and N)
    /*
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        horizontalGreen = !horizontalGreen;
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        verticalGreen = !verticalGreen;
    }
    */

    // Toggle traffic lights with Spacebar
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && !lightTogglePressed) {
        if (horizontalGreen) {
            horizontalGreen = false;
            verticalGreen = true;
        } else {
            horizontalGreen = true;
            verticalGreen = false;
        }
        lightTogglePressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE) {
        lightTogglePressed = false;
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

    // Draw background (grass)
    glColor3f(0.2f, 0.5f, 0.1f); // Green color for grass
    glBegin(GL_QUADS);
        glVertex2f(-1.0f, -1.0f); // Bottom-left
        glVertex2f(1.0f, -1.0f);  // Bottom-right
        glVertex2f(1.0f, 1.0f);   // Top-right
        glVertex2f(-1.0f, 1.0f);  // Top-left
    glEnd();

    // Draw roads
    drawRectangle(-1.0f, -0.1f, 2.0f, 0.2f, 0.2f, 0.2f, 0.2f); // Horizontal
    drawRectangle(-0.1f, -1.0f, 0.2f, 2.0f, 0.2f, 0.2f, 0.2f); // Vertical

    // Draw lane lines
    glColor3f(1.0f, 1.0f, 1.0f); // White color for lane lines
    glBegin(GL_LINES);
        // Horizontal lane lines
        glVertex2f(-1.0f, 0.0f); // Left end of middle line
        glVertex2f(1.0f, 0.0f);  // Right end of middle line

        // Vertical lane lines
        glVertex2f(0.0f, -1.0f); // Bottom end of middle line
        glVertex2f(0.0f, 1.0f);  // Top end of middle line
    glEnd();

    // Draw crosswalks
    glColor3f(1.0f, 1.0f, 1.0f); // White color for crosswalks
    glBegin(GL_QUADS);
        // Top crosswalk
        glVertex2f(-0.1f, 0.1f); glVertex2f(0.1f, 0.1f); glVertex2f(0.1f, 0.15f); glVertex2f(-0.1f, 0.15f);
        glVertex2f(-0.1f, 0.2f); glVertex2f(0.1f, 0.2f); glVertex2f(0.1f, 0.25f); glVertex2f(-0.1f, 0.25f);

        // Bottom crosswalk
        glVertex2f(-0.1f, -0.1f); glVertex2f(0.1f, -0.1f); glVertex2f(0.1f, -0.15f); glVertex2f(-0.1f, -0.15f);
        glVertex2f(-0.1f, -0.2f); glVertex2f(0.1f, -0.2f); glVertex2f(0.1f, -0.25f); glVertex2f(-0.1f, -0.25f);

        // Left crosswalk
        glVertex2f(-0.1f, -0.1f); glVertex2f(-0.15f, -0.1f); glVertex2f(-0.15f, 0.1f); glVertex2f(-0.1f, 0.1f);
        glVertex2f(-0.2f, -0.1f); glVertex2f(-0.25f, -0.1f); glVertex2f(-0.25f, 0.1f); glVertex2f(-0.2f, 0.1f);

        // Right crosswalk
        glVertex2f(0.1f, -0.1f); glVertex2f(0.15f, -0.1f); glVertex2f(0.15f, 0.1f); glVertex2f(0.1f, 0.1f);
        glVertex2f(0.2f, -0.1f); glVertex2f(0.25f, -0.1f); glVertex2f(0.25f, 0.1f); glVertex2f(0.2f, 0.1f);
    glEnd();

    // Draw simple trees (refined shapes)
    glColor3f(0.5f, 0.35f, 0.05f); // Brown color for trunks
    glBegin(GL_QUADS);
        glVertex2f(-0.7f, 0.4f); glVertex2f(-0.75f, 0.4f); glVertex2f(-0.75f, 0.55f); glVertex2f(-0.7f, 0.55f);
        glVertex2f(0.6f, -0.5f); glVertex2f(0.55f, -0.5f); glVertex2f(0.55f, -0.35f); glVertex2f(0.6f, -0.35f);
    glEnd();

    glColor3f(0.1f, 0.6f, 0.1f); // Green color for tree tops (using triangles for a more rounded look)
    glBegin(GL_TRIANGLES);
        glVertex2f(-0.725f, 0.7f); glVertex2f(-0.85f, 0.55f); glVertex2f(-0.6f, 0.55f); // Top triangle
        glVertex2f(-0.725f, 0.6f); glVertex2f(-0.8f, 0.45f); glVertex2f(-0.65f, 0.45f); // Middle triangle

        glVertex2f(0.575f, -0.2f); glVertex2f(0.45f, -0.35f); glVertex2f(0.7f, -0.35f); // Top triangle
        glVertex2f(0.575f, -0.3f); glVertex2f(0.5f, -0.45f); glVertex2f(0.65f, -0.45f); // Middle triangle
    glEnd();

    // Draw simple lampposts (simplified with small quads for now)
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow color for lamppost lights (simplified with small quads for now)
    glBegin(GL_QUADS);
        glVertex2f(-0.22f, 0.8f); glVertex2f(-0.16f, 0.8f); glVertex2f(-0.16f, 0.83f); glVertex2f(-0.22f, 0.83f);
        glVertex2f(0.16f, -0.8f); glVertex2f(0.22f, -0.8f); glVertex2f(0.22f, -0.83f); glVertex2f(0.16f, -0.83f);
    glEnd();

    // Draw buildings
    // Building 1 (Top-left)
    glColor3f(0.7f, 0.7f, 0.7f); // Wall color
    glBegin(GL_QUADS);
        glVertex2f(-0.9f, 0.5f); glVertex2f(-0.6f, 0.5f); glVertex2f(-0.6f, 0.9f); glVertex2f(-0.9f, 0.9f);
    glEnd();
    glColor3f(0.4f, 0.4f, 0.5f); // Window color
    glBegin(GL_QUADS);
        glVertex2f(-0.85f, 0.55f); glVertex2f(-0.8f, 0.55f); glVertex2f(-0.8f, 0.85f); glVertex2f(-0.85f, 0.85f);
        glVertex2f(-0.75f, 0.55f); glVertex2f(-0.7f, 0.55f); glVertex2f(-0.7f, 0.85f); glVertex2f(-0.75f, 0.85f);
    glEnd();

    // Building 2 (Bottom-right)
    glColor3f(0.8f, 0.5f, 0.5f); // Wall color
    glBegin(GL_QUADS);
        glVertex2f(0.6f, -0.9f); glVertex2f(0.9f, -0.9f); glVertex2f(0.9f, -0.5f); glVertex2f(0.6f, -0.5f);
    glEnd();
     glColor3f(0.4f, 0.5f, 0.4f); // Window color
    glBegin(GL_QUADS);
        glVertex2f(0.65f, -0.85f); glVertex2f(0.7f, -0.85f); glVertex2f(0.7f, -0.55f); glVertex2f(0.65f, -0.55f);
        glVertex2f(0.75f, -0.85f); glVertex2f(0.8f, -0.85f); glVertex2f(0.8f, -0.55f); glVertex2f(0.75f, -0.55f);
        glVertex2f(0.85f, -0.85f); glVertex2f(0.9f, -0.85f); glVertex2f(0.9f, -0.55f); glVertex2f(0.85f, -0.55f);
    glEnd();

    // Building 3 (Bottom-left, smaller)
    glColor3f(0.5f, 0.5f, 0.8f); // Wall color
    glBegin(GL_QUADS);
        glVertex2f(-0.95f, -0.95f); glVertex2f(-0.8f, -0.95f); glVertex2f(-0.8f, -0.7f); glVertex2f(-0.95f, -0.7f);
    glEnd();
     glColor3f(0.3f, 0.3f, 0.4f); // Window color
    glBegin(GL_QUADS);
        glVertex2f(-0.9f, -0.9f); glVertex2f(-0.85f, -0.9f); glVertex2f(-0.85f, -0.75f); glVertex2f(-0.9f, -0.75f);
    glEnd();

     // Building 4 (Top-right, taller)
    glColor3f(0.5f, 0.8f, 0.5f); // Wall color
    glBegin(GL_QUADS);
        glVertex2f(0.7f, 0.6f); glVertex2f(0.95f, 0.6f); glVertex2f(0.95f, 0.95f); glVertex2f(0.7f, 0.95f);
    glEnd();
     glColor3f(0.4f, 0.6f, 0.4f); // Window color
    glBegin(GL_QUADS);
        glVertex2f(0.75f, 0.65f); glVertex2f(0.8f, 0.65f); glVertex2f(0.8f, 0.9f); glVertex2f(0.75f, 0.9f);
        glVertex2f(0.85f, 0.65f); glVertex2f(0.9f, 0.65f); glVertex2f(0.9f, 0.9f); glVertex2f(0.85f, 0.9f);
    glEnd();

    // Draw traffic lights (larger)
    drawRectangle(0.3f, 0.1f, 0.1f, 0.1f, horizontalGreen ? 0.0f : 1.0f, horizontalGreen ? 1.0f : 0.0f, 0.0f); // Horizontal light
    drawRectangle(0.1f, 0.3f, 0.1f, 0.1f, verticalGreen ? 0.0f : 1.0f, verticalGreen ? 1.0f : 0.0f, 0.0f); // Vertical light

    // Draw horizontal cars (larger)
    for (auto& car : horizontalCars) {
        glColor3f(1.0f, 0.0f, 0.0f); // Red color for horizontal cars
        glBegin(GL_QUADS); // Using QUADS for a slightly more complex shape
            // Main body
            glVertex2f(car.x, car.y + 0.02f); // Bottom-left of main body
            glVertex2f(car.x + 0.15f, car.y + 0.02f); // Bottom-right of main body
            glVertex2f(car.x + 0.15f, car.y + 0.08f); // Top-right of main body
            glVertex2f(car.x, car.y + 0.08f); // Top-left of main body

            // Front (hood)
            glVertex2f(car.x + 0.15f, car.y + 0.03f);
            glVertex2f(car.x + 0.18f, car.y + 0.03f);
            glVertex2f(car.x + 0.18f, car.y + 0.07f);
            glVertex2f(car.x + 0.15f, car.y + 0.07f);

            // Back (trunk)
            glVertex2f(car.x - 0.03f, car.y + 0.03f);
            glVertex2f(car.x, car.y + 0.03f);
            glVertex2f(car.x, car.y + 0.07f);
            glVertex2f(car.x - 0.03f, car.y + 0.07f);
        glEnd();
    }

    // Draw vertical cars (larger)
    for (auto& car : verticalCars) {
        glColor3f(0.0f, 0.0f, 1.0f); // Blue color for vertical cars
         glBegin(GL_QUADS); // Using QUADS for a slightly more complex shape
            // Main body
            glVertex2f(car.x + 0.02f, car.y); // Bottom-left of main body
            glVertex2f(car.x + 0.08f, car.y); // Bottom-right of main body
            glVertex2f(car.x + 0.08f, car.y - 0.15f); // Top-right of main body
            glVertex2f(car.x + 0.02f, car.y - 0.15f); // Top-left of main body

            // Front (hood)
            glVertex2f(car.x + 0.03f, car.y - 0.15f);
            glVertex2f(car.x + 0.07f, car.y - 0.15f);
            glVertex2f(car.x + 0.07f, car.y - 0.18f);
            glVertex2f(car.x + 0.03f, car.y - 0.18f);

            // Back (trunk)
            glVertex2f(car.x + 0.03f, car.y + 0.03f);
            glVertex2f(car.x + 0.07f, car.y + 0.03f);
            glVertex2f(car.x + 0.07f, car.y);
            glVertex2f(car.x + 0.03f, car.y);
        glEnd();
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
            // Collision happens if the front of the current car is at or past the back of the car ahead, minus the desired gap.
            // Current horizontal car front is approximately car.x + 0.18f.
            // Horizontal car ahead back is approximately horizontalCars[i-1].x - 0.03f.
            if (horizontalCars[i].x + 0.18f >= horizontalCars[i-1].x - 0.03f - DESIRED_CAR_GAP) { 
                canMove = false;
            }
        }

        // Check for traffic light
        if (!horizontalGreen) {
            // Stop line for horizontal cars moving right is at x = -0.1
            // Stop if the front of the car is at or past the stop line minus the desired gap
            // Horizontal car front is approx car.x + 0.18f
            if (horizontalCars[i].x + 0.18f >= -0.1f - DESIRED_CAR_GAP) {
            canMove = false;
            }
        }

        if (canMove) {
            horizontalCars[i].x += horizontalCars[i].speed * simulationSpeed;
        }
    }

    // Remove horizontal cars that are off-screen
    horizontalCars.erase(
        std::remove_if(horizontalCars.begin(), horizontalCars.end(),
                       [](const Car& car) {
                           return car.x > 1.2f; // Remove if moved far right
                       }),
        horizontalCars.end());

    // Update vertical cars
    for (size_t i = 0; i < verticalCars.size(); ++i) {
        bool canMove = true;
        // Check for collision with the car ahead
        if (i > 0) {
            // Collision happens if the front (bottom) of the current car is at or past the back (top) of the car ahead, plus the desired gap.
            // Current vertical car front (bottom) is approximately car.y - 0.15f.
            // Vertical car ahead back (top) is approximately verticalCars[i-1].y + 0.03f.
             if (verticalCars[i].y - 0.15f <= verticalCars[i-1].y + 0.03f + DESIRED_CAR_GAP) { 
                canMove = false;
            }
        }

        // Check for traffic light
        if (!verticalGreen) {
            // Stop line for vertical cars moving down is at y = 0.1
            // Stop if the front (bottom) of the car is at or past the stop line plus the desired gap
            // Vertical car front (bottom) is approx car.y - 0.15f
            if (verticalCars[i].y - 0.15f <= 0.1f + DESIRED_CAR_GAP) {
             canMove = false;
            }
        }

        if (canMove) {
            verticalCars[i].y -= verticalCars[i].speed * simulationSpeed; // Negative because moving down
        }
    }

    // Remove vertical cars that are off-screen
    verticalCars.erase(
        std::remove_if(verticalCars.begin(), verticalCars.end(),
                       [](const Car& car) {
                           return car.y < -1.2f; // Remove if moved far down
                       }),
        verticalCars.end());
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
                // Generate a random speed
                float randomSpeed = carSpeedDist(rng);
                if (carType == 0) { // Horizontal car
                    // Initialize currentSpeed to 0.0f, assign random max speed
                    horizontalCars.push_back({-0.95f, -0.05f, randomSpeed, 0.0f, 0});
                } else { // Vertical car
                    // Initialize currentSpeed to 0.0f, assign random max speed
                    verticalCars.push_back({-0.05f, 0.95f, randomSpeed, 0.0f, 1});
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