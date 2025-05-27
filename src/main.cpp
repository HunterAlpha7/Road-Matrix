#include <glad.h>
#include <glfw3.h>
#include <bits/stdc++.h>
#include <random>
#include <chrono>

struct Car {
    float x, y;
    float speed;
    int direction; // 0 = horizontal, 1 = vertical
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