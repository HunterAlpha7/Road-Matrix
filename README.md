# 🚦 Traffic Simulation Project ✨

Welcome to the Traffic Simulation project! This program creates a simple visual simulation of cars moving through an intersection with a traffic light.

Think of it like a miniature world where we can watch cars drive around and respond to a signal.

## 🌟 Key Features (How it Works in Simple Terms) 🌟

Here are the main things happening in this simulation:

### 🚗 1. Cars Appear Randomly

Instead of making cars appear manually, new cars pop up on the roads by themselves every now and then, like cars joining a real road. Some cars go sideways (horizontal) and some go up and down (vertical).

### 🚥 2. Traffic Lights Control Flow

There's a traffic light at the center. We can change the lights for both roads using **one button: the 'A' key** on your keyboard.

*   When you press 'A', the lights will swap. If the horizontal road has a green light, the vertical road will have a red light, and vice versa.
*   This makes sure cars from different directions don't crash in the middle!

### ✅ 3. Cars Move and Stop Safely

*   Cars drive along their roads at different speeds (some are a bit faster than others!).
*   They are programmed to **stop** if there is a red light ahead of them.
*   They also **stop** if they get too close to the car right in front of them. This prevents them from bumping into each other.
*   Once they drive far away off the screen, they are removed from the simulation to keep things running smoothly.

### 🏙️ 4. What You See On Screen

The simulation shows a simple intersection with:

*   **Roads:** The grey areas where cars drive.
*   **Lane Lines:** White lines on the roads to show lanes.
*   **Traffic Lights:** Colored boxes that change between red and green (controlled by the 'A' key).
*   **Cars:** Simple shapes representing vehicles, colored red for horizontal and blue for vertical.
*   **Background Pictures:** Two images placed near the traffic lights.
*   **Scenery:** Simple shapes representing trees and lampposts, and some buildings in the background corners.

### ⏩⏪ 5. Controlling the Simulation Speed

You can make the cars move faster or slower using your keyboard:

*   Press the **Up Arrow key** to speed up the simulation.
*   Press the **Down Arrow key** to slow down the simulation.

---
## 🏃‍♀️ Running the Project

```g++.exe -fdiagnostics-color=always -I./include ./src/main.cpp ./src/glad.c -o ./build/main.exe -Llib -lglfw3 -lopengl32 -lgdi32 ./build/main.exe```

---

This simulation provides a basic visual example of how traffic can be managed at an intersection using simple rules for car movement and traffic light control. It shows how different elements in a programmed world can interact with each other. 
