#include <glad.h>
#include <glfw3.h>

#include <bits/stdc++.h>
// Window settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

// Vertex shader (simple pass-through)
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;

uniform vec2 offset;
uniform vec2 scale;

void main()
{
    vec2 pos = aPos * scale + offset;
    gl_Position = vec4(pos, 0.0, 1.0);
}
)";

// Fragment shader with uniform color
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 color;

void main()
{
    FragColor = vec4(color, 1.0);
}
)";

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Traffic Light state
enum TrafficLightState { RED, GREEN };
TrafficLightState lightState = RED;

// Simulation speed multiplier
float simSpeed = 1.0f;

// Simple 2D rectangle shape (unit square centered at origin)
float rectVertices[] = {
    -0.05f, -0.05f,
     0.05f, -0.05f,
     0.05f, 0.05f,
    -0.05f, 0.05f
};

unsigned int rectIndices[] = {
    0,1,2,
    2,3,0
};

// Car struct with position and direction
struct Car {
    float x, y;
    float speed;
    bool horizontal; // true if moves horizontally, else vertically
    bool forward; // direction (true = right/up, false = left/down)
};

// Container for cars
std::vector<Car> cars;

// GLFW window pointer
GLFWwindow* window;

// Shader program ID
unsigned int shaderProgram;

// VAO, VBO, EBO for rectangle
unsigned int VAO, VBO, EBO;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0,0,width,height);
}

void processInput(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
        glfwSetWindowShouldClose(window,true);

    static bool spacePressedLast = false;
    bool spacePressedNow = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    // Toggle traffic light on Space press (rising edge)
    if(spacePressedNow && !spacePressedLast)
    {
        if(lightState == RED) lightState = GREEN;
        else lightState = RED;
    }
    spacePressedLast = spacePressedNow;

    static bool cPressedLast = false;
    bool cPressedNow = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
    if(cPressedNow && !cPressedLast)
    {
        // Add new car at start of horizontal lane (left to right)
        cars.push_back(Car{-0.95f, 0.1f, 0.3f, true, true});
    }
    cPressedLast = cPressedNow;

    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        simSpeed += 0.5f * deltaTime;
        if(simSpeed > 5.0f) simSpeed = 5.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        simSpeed -= 0.5f * deltaTime;
        if(simSpeed < 0.1f) simSpeed = 0.1f;
    }
}

unsigned int compileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader,1,&source,nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader,GL_COMPILE_STATUS,&success);
    if(!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader,512,nullptr,infoLog);
        std::cerr << "Shader compilation error:\n" << infoLog << std::endl;
    }
    return shader;
}

unsigned int createShaderProgram()
{
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER,vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER,fragmentShaderSource);

    unsigned int program = glCreateProgram();
    glAttachShader(program,vertexShader);
    glAttachShader(program,fragmentShader);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program,GL_LINK_STATUS,&success);
    if(!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program,512,nullptr,infoLog);
        std::cerr << "Shader linking error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void initRectangle()
{
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(rectVertices),rectVertices,GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(rectIndices),rectIndices,GL_STATIC_DRAW);

    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

// Draw a rectangle at given position, scale and color
void drawRect(float x, float y, float scaleX, float scaleY, float r, float g, float b)
{
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    int offsetLoc = glGetUniformLocation(shaderProgram,"offset");
    int scaleLoc = glGetUniformLocation(shaderProgram,"scale");
    int colorLoc = glGetUniformLocation(shaderProgram,"color");

    glUniform2f(offsetLoc,x,y);
    glUniform2f(scaleLoc,scaleX,scaleY);
    glUniform3f(colorLoc,r,g,b);

    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

    glBindVertexArray(0);
}

void updateCars(float dt)
{
    for(auto& car : cars)
    {
        if(lightState == RED)
        {
            // Stop cars near intersection center (within 0.15 units)
            if(car.horizontal)
            {
                if(car.forward && car.x >= -0.15f) continue; // stop before center
                if(!car.forward && car.x <= 0.15f) continue;
            }
            else
            {
                if(car.forward && car.y >= -0.15f) continue;
                if(!car.forward && car.y <= 0.15f) continue;
            }
        }

        // Move car
        float moveDist = car.speed * dt * simSpeed;
        if(car.horizontal)
        {
            car.x += (car.forward ? moveDist : -moveDist);
            // Reset if out of bounds
            if(car.forward && car.x > 1.1f) car.x = -1.1f;
            if(!car.forward && car.x < -1.1f) car.x = 1.1f;
        }
        else
        {
            car.y += (car.forward ? moveDist : -moveDist);
            if(car.forward && car.y > 1.1f) car.y = -1.1f;
            if(!car.forward && car.y < -1.1f) car.y = 1.1f;
        }
    }
}

int main()
{
    // Initialize GLFW
    if(!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
#endif

    window = glfwCreateWindow(SCR_WIDTH,SCR_HEIGHT,"Traffic Simulation",NULL,NULL);
    if(!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);

    // Load GLAD
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Compile shaders and create program
    shaderProgram = createShaderProgram();

    // Setup rectangle data (cars, roads)
    initRectangle();

    // Add initial cars: horizontal right and vertical up
    cars.push_back(Car{-0.9f, 0.1f, 0.3f, true, true});
    cars.push_back(Car{0.1f, -0.9f, 0.25f, false, true});

    // Timing variables
    float currentFrame, lastFrameTime = 0.0f;

    // Main loop
    while(!glfwWindowShouldClose(window))
    {
        // Calculate deltaTime
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrameTime;
        lastFrameTime = currentFrame;

        processInput(window);

        // Update car positions
        updateCars(deltaTime);

        // Clear background (gray)
        glClearColor(0.2f,0.2f,0.2f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw intersection roads - two big rectangles
        // Horizontal road (dark gray)
        drawRect(0.0f, 0.0f, 1.0f, 0.25f, 0.1f,0.1f,0.1f);
        // Vertical road (dark gray)
        drawRect(0.0f, 0.0f, 0.25f, 1.0f, 0.1f,0.1f,0.1f);

        // Draw traffic light squares at intersection corners
        // Let's put it top-left corner, size 0.05x0.05
        if(lightState == RED)
            drawRect(-0.2f, 0.2f, 0.05f, 0.05f, 1.0f,0.0f,0.0f);
        else
            drawRect(-0.2f, 0.2f, 0.05f, 0.05f, 0.0f,1.0f,0.0f);

        // Draw cars (blue rectangles for horizontal, orange for vertical)
        for(const auto& car : cars)
        {
            if(car.horizontal)
                drawRect(car.x, car.y, 0.1f, 0.05f, 0.0f,0.0f,1.0f);
            else
                drawRect(car.x, car.y, 0.05f, 0.1f, 1.0f,0.5f,0.0f);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1,&VAO);
    glDeleteBuffers(1,&VBO);
    glDeleteBuffers(1,&EBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}