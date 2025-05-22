#include <glad.h>
#include <glfw3.h>
#include <bits/stdc++.h>

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

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        horizontalCars.push_back({-0.95f, -0.05f, 0.005f, 0});
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        verticalCars.push_back({-0.05f, 0.95f, 0.005f, 1});
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
    drawRectangle(0.7f, 0.1f, 0.1f, 0.1f, horizontalGreen ? 0.0f : 1.0f, horizontalGreen ? 1.0f : 0.0f, 0.0f); // Horizontal light
    drawRectangle(0.1f, 0.7f, 0.1f, 0.1f, verticalGreen ? 0.0f : 1.0f, verticalGreen ? 1.0f : 0.0f, 0.0f); // Vertical light

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
    for (auto& car : horizontalCars) {
        if (horizontalGreen || car.x + 0.15f < -0.1f || car.x > 0.1f)
            car.x += car.speed * simulationSpeed;
    }

    for (auto& car : verticalCars) {
        if (verticalGreen || car.y > 0.1f || car.y + 0.15f < -0.1f)
            car.y -= car.speed * simulationSpeed;
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
        renderScene();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}